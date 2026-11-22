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
#include <stdlib.h>
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

/*
 * OpenSSL functions libssladp.so needs that M's BoringSSL dropped (L's
 * BoringSSL still had them). Without them mtk_agpsd fails to link.
 *
 * BoringSSL's OPENSSL_malloc/OPENSSL_free are plain malloc/free, so the
 * CRYPTO_* wrappers are exact. SSLv2 is gone: hand out the SSLv23 method
 * (negotiates TLS). PKCS7/PKCS12 (client certificate from a .p12) fail
 * cleanly: NULL / 0, the free functions accept only NULL.
 */
void *CRYPTO_malloc(int num, const char *file, int line)
{
    (void)file; (void)line;
    return num > 0 ? malloc(num) : NULL;
}

void CRYPTO_free(void *ptr) { free(ptr); }

void OPENSSL_add_all_algorithms_noconf(void) { }

static void *ssl_method(const char *name)
{
    method_fn fn = (method_fn)dlsym(RTLD_DEFAULT, name);

    if (!fn) {
        __android_log_print(ANDROID_LOG_ERROR, "mtkshim", "%s not found", name);
        return NULL;
    }
    return fn();
}

void *SSLv2_client_method(void) { return ssl_method("SSLv23_client_method"); }
void *SSLv2_server_method(void) { return ssl_method("SSLv23_server_method"); }

static void pkcs_unsupported(const char *fn)
{
    __android_log_print(ANDROID_LOG_WARN, "mtkshim", "%s: no PKCS7/PKCS12 in BoringSSL", fn);
}

static void pkcs_free(const char *fn, void *p)
{
    if (p)
        __android_log_print(ANDROID_LOG_ERROR, "mtkshim", "%s: unexpected non-NULL object, leaked", fn);
}

void *d2i_PKCS7_bio(void *bio, void **p7)
{
    (void)bio; (void)p7;
    pkcs_unsupported(__func__);
    return NULL;
}

void PKCS7_free(void *p7) { pkcs_free(__func__, p7); }
void PKCS12_SAFEBAG_free(void *bag) { pkcs_free(__func__, bag); }

void *PKCS12_unpack_authsafes(void *p12)
{
    (void)p12;
    pkcs_unsupported(__func__);
    return NULL;
}

void *PKCS12_unpack_p7data(void *p7)
{
    (void)p7;
    pkcs_unsupported(__func__);
    return NULL;
}

void *PKCS12_unpack_p7encdata(void *p7, const char *pass, int passlen)
{
    (void)p7; (void)pass; (void)passlen;
    pkcs_unsupported(__func__);
    return NULL;
}

void *PKCS12_decrypt_skey(void *bag, const char *pass, int passlen)
{
    (void)bag; (void)pass; (void)passlen;
    pkcs_unsupported(__func__);
    return NULL;
}

void *PKCS12_certbag2x509(void *bag)
{
    (void)bag;
    pkcs_unsupported(__func__);
    return NULL;
}

int PKCS12_verify_mac(void *p12, const char *pass, int passlen)
{
    (void)p12; (void)pass; (void)passlen;
    pkcs_unsupported(__func__);
    return 0;
}
