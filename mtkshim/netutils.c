/*
 * MTK libnetutils extra: int ifc_set_throttle(const char *ifname, int rxKbps, int txKbps).
 * Called by libbwc.so (BWC::modem_speed_profile_set, via libcamdrv.so) to
 * limit modem bandwidth while the camera records. Not needed; report success.
 */
#include <android/log.h>

int ifc_set_throttle(const char *ifname, int rxKbps, int txKbps)
{
    __android_log_print(ANDROID_LOG_DEBUG, "mtkshim", "ifc_set_throttle(%s, %d, %d) ignored",
                        ifname ? ifname : "(null)", rxKbps, txKbps);
    return 0;
}
