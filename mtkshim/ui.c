/*
 * KK libui/libutils symbols that the camera blobs (libcam.client,
 * libcam.camadapter) and hwcomposer.mt8389.so still import:
 *
 * - RefBase::moveReferences(): JB MR1 dropped it; CM 12.1 kept an empty
 *   stub under REFBASE_JB_MR1_COMPAT_SYMBOLS, CM 13 has none. Without
 *   DEBUG_REFS it only renamed debug ref ids, so a no-op is correct.
 *
 * - GraphicBufferMapper::lock(handle, int usage, ...): M made usage a
 *   uint32_t (mangled ...iRK... -> ...jRK...). Same ABI: forward.
 *
 * - GraphicBufferMapper::getIonFd(): MTK addition (lenovo-a3 cm-12.1
 *   frameworks/native b31942c + 1c3c1f2). Instead of forking libui, open
 *   gralloc.mt8389's "extra" device here and call its getIonFd. `this` is
 *   unused: libui's GraphicBufferMapper has no MTK member in M, and the
 *   blobs only reach the object through the Singleton.
 *
 * Everything outside libc/libdl/liblog is looked up with dlsym, as in the
 * rest of libmtkshim (it is loaded into every process).
 */
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <android/log.h>
#include <hardware/hardware.h>

#define TAG "mtkshim"

/* android::RefBase::moveReferences(void*, void const*, size_t, ReferenceConverterBase const&) */
void _ZN7android7RefBase14moveReferencesEPvPKvjRKNS_22ReferenceConverterBaseE(
        void *dst, const void *src, size_t n, const void *caster)
{
    (void)dst; (void)src; (void)n; (void)caster;
}

/* android::GraphicBufferMapper::lock(buffer_handle_t, int, Rect const&, void**) */
typedef int (*gbm_lock_fn)(void *self, const void *handle, uint32_t usage,
                           const void *bounds, void **vaddr);

int _ZN7android19GraphicBufferMapper4lockEPK13native_handleiRKNS_4RectEPPv(
        void *self, const void *handle, int usage, const void *bounds, void **vaddr)
{
    static gbm_lock_fn fn;

    if (!fn)
        fn = (gbm_lock_fn)dlsym(RTLD_DEFAULT,
                "_ZN7android19GraphicBufferMapper4lockEPK13native_handlejRKNS_4RectEPPv");
    if (!fn) {
        __android_log_print(ANDROID_LOG_ERROR, TAG,
                            "GraphicBufferMapper::lock(..., uint32_t, ...) not found");
        return -ENOSYS;
    }
    return fn(self, handle, (uint32_t)usage, bounds, vaddr);
}

/* gralloc.mt8389.so "extra" device (KK hardware/libhardware gralloc.h, MTK_MT6589) */
struct extra_device_t {
    struct hw_device_t common;
    int (*getIonFd)(struct extra_device_t *dev, const void *handle, int *idx, int *num);
    void *reserved_proc[7];
};

static struct extra_device_t *extra_dev;
static pthread_once_t extra_once = PTHREAD_ONCE_INIT;

static void extra_open(void)
{
    typedef int (*get_module_fn)(const char *id, const struct hw_module_t **module);
    const struct hw_module_t *module;
    get_module_fn get_module;
    void *lib;
    int err;

    lib = dlopen("libhardware.so", RTLD_NOW);
    get_module = lib ? (get_module_fn)dlsym(lib, "hw_get_module") : NULL;
    if (!get_module) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "hw_get_module not found");
        return;
    }
    err = get_module("gralloc", &module);
    if (err == 0)
        err = module->methods->open(module, "extra", (struct hw_device_t **)&extra_dev);
    if (err != 0) {
        __android_log_print(ANDROID_LOG_WARN, TAG, "gralloc extra device not available (%d)", err);
        extra_dev = NULL;
    }
}

/* android::GraphicBufferMapper::getIonFd(buffer_handle_t, int*, int*) */
int _ZN7android19GraphicBufferMapper8getIonFdEPK13native_handlePiS4_(
        void *self, const void *handle, int *idx, int *num)
{
    int err;

    (void)self;
    pthread_once(&extra_once, extra_open);
    if (!extra_dev || !extra_dev->getIonFd) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "gralloc extra device is not supported");
        return -ENOSYS; /* INVALID_OPERATION */
    }
    err = extra_dev->getIonFd(extra_dev, handle, idx, num);
    if (err)
        __android_log_print(ANDROID_LOG_WARN, TAG, "getIonFd(...) failed %d", err);
    return err;
}
