/*
 * MTK AudioSystem methods that audio.primary.mt6589.so calls (voice unlock
 * and ATV). Not in AOSP libmedia; cm-11.0 stubbed them the same way in
 * frameworks/av f9a45a85. Everything returns 0 / NULL (the original
 * returned "void" from some, leaving r0 undefined).
 */
#include <stddef.h>
#include <stdint.h>

int _ZN7android11AudioSystem24getVoiceUnlockDLInstanceEv(void) { return 0; }
int _ZN7android11AudioSystem23GetVoiceUnlockDLLatencyEv(void) { return 0; }
int _ZN7android11AudioSystem17SetVoiceUnlockSRCEjj(uint32_t a, uint32_t b)
{ (void)a; (void)b; return 0; }
int _ZN7android11AudioSystem18startVoiceUnlockDLEv(void) { return 0; }
int _ZN7android11AudioSystem15ReadRefFromRingEPvjS1_(void *buf, uint32_t len, void *ts)
{ (void)buf; (void)len; (void)ts; return 0; }
int _ZN7android11AudioSystem20GetVoiceUnlockULTimeEPv(void *ts) { (void)ts; return 0; }
int _ZN7android11AudioSystem25freeVoiceUnlockDLInstanceEv(void) { return 0; }
int _ZN7android11AudioSystem17stopVoiceUnlockDLEv(void) { return 0; }

/*
 * sp<IATVCtrlService> IATVCtrlService::asInterface(const sp<IBinder>&)
 * sp<> has a non-trivial copy ctor/dtor, so (Itanium C++ ABI) it is
 * returned through a hidden pointer in r0 and the argument moves to r1.
 * Return a null sp: just store NULL, no refcounting needed.
 */
void _ZN7android15IATVCtrlService11asInterfaceERKNS_2spINS_7IBinderEEE(void **ret,
                                                                       const void *binder)
{
    (void)binder;
    *ret = NULL;
}
