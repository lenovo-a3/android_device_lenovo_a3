/*
 * libshim_rilmtk: LD_PRELOADed for rild only (init.modem.rc).
 *
 * librilmtk.so was built against KK's Parcel (48 bytes). M added
 * size_t mOpenAshmemSize at the end (52 bytes), and Parcel() zeroes it in
 * initState(). librilmtk keeps Parcels on the stack, so the constructor
 * wrote past the object: in processCommandBuffer() that is the saved r4,
 * which holds processCommandsCallback()'s RecordStream, and the next
 * record_stream_get_next(NULL, ...) crashed rild with SIGSEGV after the
 * first request.
 *
 * For Parcels constructed from librilmtk, keep the 4 bytes after the KK
 * object as they were. M only writes mOpenAshmemSize in initState() (the
 * constructor, freeData(); librilmtk's Parcels have no owner, so setData()
 * doesn't get there) and for ashmem fds, which RIL parcels don't carry.
 * Parcels constructed by anything else get the plain M constructor.
 */
#include <dlfcn.h>
#include <stdint.h>
#include <string.h>
#include <android/log.h>

#define KK_PARCEL_SIZE 48

typedef void (*parcel_ctor_fn)(void *parcel);

static parcel_ctor_fn real_ctor;
static void *rilmtk_base;

static void init_once(void)
{
    Dl_info info;
    void *sym;

    real_ctor = (parcel_ctor_fn)dlsym(RTLD_NEXT, "_ZN7android6ParcelC1Ev");
    sym = dlsym(RTLD_DEFAULT, "RIL_register");
    if (sym && dladdr(sym, &info) && strstr(info.dli_fname, "librilmtk.so"))
        rilmtk_base = info.dli_fbase;
    if (!real_ctor || !rilmtk_base)
        __android_log_print(ANDROID_LOG_ERROR, "mtkshim",
                            "rilmtk: Parcel() %p, librilmtk base %p", real_ctor, rilmtk_base);
}

static int called_from_rilmtk(const void *ret)
{
    Dl_info info;

    return rilmtk_base && dladdr(ret, &info) && info.dli_fbase == rilmtk_base;
}

void _ZN7android6ParcelC1Ev(void *parcel)
{
    const void *ret = __builtin_return_address(0);
    uint32_t saved;
    int kk;

    if (!real_ctor)
        init_once();
    kk = called_from_rilmtk(ret);
    if (kk)
        memcpy(&saved, (char *)parcel + KK_PARCEL_SIZE, sizeof(saved));
    real_ctor(parcel);
    if (kk)
        memcpy((char *)parcel + KK_PARCEL_SIZE, &saved, sizeof(saved));
}
