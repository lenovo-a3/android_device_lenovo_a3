/*
 * KK libutils android::Tracer (removed in L; tracing moved to libcutils
 * atrace). Used by hwcomposer.mt8389.so and libdpframework_os.so.
 * Same values as cm-11.0's MediatekHacks.cpp (8e0727cf): "ready", no tags
 * enabled, so the blobs never try to write trace markers.
 */
#include <stdint.h>

void _ZN7android6Tracer4initEv(void) {}            /* Tracer::init() */
volatile int32_t _ZN7android6Tracer8sIsReadyE = 1;      /* Tracer::sIsReady */
uint64_t _ZN7android6Tracer12sEnabledTagsE = 0;         /* Tracer::sEnabledTags */
int _ZN7android6Tracer8sTraceFDE = -1;                  /* Tracer::sTraceFD */
