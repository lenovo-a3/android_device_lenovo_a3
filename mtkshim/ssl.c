/*
 * MTK OpenSSL extra used by libssladp.so (mtk_agpsd, SUPL TLS):
 *   int X509_STORE_load_locations_ext(X509_STORE *ctx, const char *file,
 *                                     const char *path, int type);
 * Like X509_STORE_load_locations(), but with the file type
 * (libssladp passes 1 = X509_FILETYPE_PEM or 2 = X509_FILETYPE_ASN1).
 * Returns 1 on success, 0 on failure.
 *
 * Implemented with the generic lookup API, looked up with dlsym so this
 * library doesn't link libcrypto (mtk_agpsd loads it).
 */
#include <dlfcn.h>
#include <stddef.h>
#include <android/log.h>

#define X509_L_FILE_LOAD 1
#define X509_L_ADD_DIR   2

typedef void *(*method_fn)(void);
typedef void *(*add_lookup_fn)(void *store, void *method);
typedef int (*lookup_ctrl_fn)(void *lookup, int cmd, const char *argc, long argl, char **ret);

int X509_STORE_load_locations_ext(void *ctx, const char *file, const char *path, int type)
{
    add_lookup_fn add_lookup = (add_lookup_fn)dlsym(RTLD_DEFAULT, "X509_STORE_add_lookup");
    method_fn lookup_file = (method_fn)dlsym(RTLD_DEFAULT, "X509_LOOKUP_file");
    method_fn lookup_dir = (method_fn)dlsym(RTLD_DEFAULT, "X509_LOOKUP_hash_dir");
    lookup_ctrl_fn ctrl = (lookup_ctrl_fn)dlsym(RTLD_DEFAULT, "X509_LOOKUP_ctrl");
    void *lookup;

    if (!add_lookup || !lookup_file || !lookup_dir || !ctrl) {
        __android_log_print(ANDROID_LOG_ERROR, "mtkshim",
                            "X509_STORE_load_locations_ext: libcrypto symbols not found");
        return 0;
    }
    if (file) {
        lookup = add_lookup(ctx, lookup_file());
        if (!lookup || ctrl(lookup, X509_L_FILE_LOAD, file, type, NULL) != 1)
            return 0;
    }
    if (path) {
        lookup = add_lookup(ctx, lookup_dir());
        if (!lookup || ctrl(lookup, X509_L_ADD_DIR, path, type, NULL) != 1)
            return 0;
    }
    return (file || path) ? 1 : 0;
}
