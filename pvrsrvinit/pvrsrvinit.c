/*
 * PIE replacement for the non-PIE KK blob pvrsrvctl (see Android.mk).
 * Equivalent to "pvrsrvctl --start --no-module".
 */

#define LOG_TAG "pvrsrvinit"

#include <dlfcn.h>
#include <stdio.h>

#include <cutils/log.h>

typedef int (*srv_init_fn)(void);

int main(void)
{
    void *lib;
    srv_init_fn srv_init;
    int err;

    lib = dlopen("libsrv_init.so", RTLD_NOW);
    if (!lib) {
        ALOGE("dlopen libsrv_init.so: %s", dlerror());
        fprintf(stderr, "dlopen libsrv_init.so: %s\n", dlerror());
        return 1;
    }

    srv_init = (srv_init_fn)dlsym(lib, "SrvInit");
    if (!srv_init) {
        ALOGE("dlsym SrvInit: %s", dlerror());
        fprintf(stderr, "dlsym SrvInit: %s\n", dlerror());
        return 1;
    }

    /* PVRSRV_OK == 0 */
    err = srv_init();
    if (err) {
        ALOGE("SrvInit failed (already initialized?) (err=%d)", err);
        fprintf(stderr, "SrvInit failed (already initialized?) (err=%d)\n", err);
        return 1;
    }

    ALOGI("PowerVR services initialised");
    return 0;
}
