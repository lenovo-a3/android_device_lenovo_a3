/*
 * android::AwesomePlayer::mtk_omx_get_current_time(int64_t*) for
 * libMtkOmxVdec.so. MTK's KK AwesomePlayer gave the decoder a pointer to
 * itself, and MtkOmxVdec::DecodeVideo then asks it for the A/V sync time
 * (passed to the driver as an AV-sync hint). M has no AwesomePlayer, so
 * nothing sets that pointer and DecodeVideo never makes the call (it checks
 * the pointer first); the symbol only has to exist for the library to load.
 * If it is ever reached, report "no time" (-1), as AwesomePlayer did for the
 * first frame and while seeking.
 */
#include <stdint.h>

void _ZN7android13AwesomePlayer24mtk_omx_get_current_timeEPx(void *self, int64_t *real_time)
{
    (void)self;
    if (real_time)
        *real_time = -1;
}
