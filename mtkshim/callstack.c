/*
 * KK android::CallStack::dump(const char* prefix) const, gone in L.
 *
 * Its only callers (libcam.utils.so MtkCamUtils::dumpCallStack(),
 * libdpframework_os.so DpOS_Android::dumpCallStack()) do:
 *     CallStack cs; cs.update(1, 31); cs.dump(prefix);
 * The CallStack object is built by L's constructor, so it has the L layout.
 * KK's update(ignoreDepth, maxDepth) mangles the same as L's
 * update(ignoreDepth, tid), so that call unwinds "thread 31", which fails
 * harmlessly (tgkill ESRCH, one warning). We therefore unwind the current
 * thread again here and print with L's CallStack::log().
 *
 * L's functions are looked up with dlsym so this library doesn't pull
 * libutils into every process; the callers link libutils themselves.
 */
#include <dlfcn.h>
#include <stdint.h>
#include <android/log.h>

#define LOG_TAG "mtkshim"

typedef void (*callstack_update_t)(void *self, int32_t ignoreDepth, int32_t tid);
typedef void (*callstack_log_t)(const void *self, const char *logtag,
                                int priority, const char *prefix);

void _ZNK7android9CallStack4dumpEPKc(const void *self, const char *prefix)
{
    static callstack_update_t update;
    static callstack_log_t log;

    if (!update)
        update = (callstack_update_t)dlsym(RTLD_DEFAULT, "_ZN7android9CallStack6updateEii");
    if (!log)
        log = (callstack_log_t)dlsym(RTLD_DEFAULT,
                "_ZNK7android9CallStack3logEPKc19android_LogPriorityS2_");
    if (!update || !log) {
        __android_log_print(ANDROID_LOG_WARN, LOG_TAG, "CallStack::dump: libutils not loaded");
        return;
    }
    /* ignore this frame and the blob's dumpCallStack(); -1 = BACKTRACE_CURRENT_THREAD */
    update((void *)self, 2, -1);
    log(self, "CallStack", ANDROID_LOG_DEBUG, prefix ? prefix : "");
}
