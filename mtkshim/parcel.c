/*
 * Parcel::writeString16(const uint16_t*, size_t), used by librilmtk.so.
 * M changed the parameter to const char16_t*, so the mangled name changed
 * (EPKtj -> EPKDsj) and rild failed to link. Same ABI: forward to the M
 * function, looked up with dlsym so this library doesn't link libbinder.
 */
#include <dlfcn.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <android/log.h>

typedef int (*write_string16_fn)(void *parcel, const uint16_t *str, size_t len);

int _ZN7android6Parcel13writeString16EPKtj(void *parcel, const uint16_t *str, size_t len)
{
    static write_string16_fn fn;

    if (!fn)
        fn = (write_string16_fn)dlsym(RTLD_DEFAULT, "_ZN7android6Parcel13writeString16EPKDsj");
    if (!fn) {
        __android_log_print(ANDROID_LOG_ERROR, "mtkshim",
                            "Parcel::writeString16(const char16_t*, size_t) not found");
        return -ENOSYS;
    }
    return fn(parcel, str, len);
}
