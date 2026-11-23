/*
 * MTK xlog, used by nearly every MT6589 blob (GPU libsrv_*, hwcomposer,
 * audio, camera, GPS, codecs, ...):
 *   void __xlog_buf_printf(int bufid, const struct xlog_record *rec, ...);
 *
 * cm-12.1 and cm-13.0 had it in liblog under MTK_HARDWARE (logd_write.c,
 * weak). N's liblog is a Soong module and has no MTK code, so without this
 * the blobs fail to load. Same behavior as M: every record goes to the main
 * log (M's property checks were behind HAVE_LIBC_SYSTEM_PROPERTIES, which
 * nothing defined).
 */
#include <stdarg.h>
#include <android/log.h>

struct xlog_record {
    const char *tag_str;
    const char *fmt_str;
    int prio;
};

void __attribute__((weak)) __xlog_buf_printf(int bufid, const struct xlog_record *rec, ...)
{
    va_list args;

    (void)bufid;
    if (!rec || !rec->fmt_str)
        return;
    va_start(args, rec);
    __android_log_vprint(rec->prio, rec->tag_str, rec->fmt_str, args);
    va_end(args);
}
