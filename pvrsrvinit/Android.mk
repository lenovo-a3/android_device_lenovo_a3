#
# pvrsrvinit: PIE replacement for the KK blob /system/vendor/bin/pvrsrvctl.
#
# The blob is a non-PIE executable (ET_EXEC), and the cm-12.1 linker refuses
# those ("only position independent executables (PIE) are supported"), so
# SGX services were never initialised and SurfaceFlinger looped on
# PVRSRVConnect. init.mt8389.rc already insmods pvrsrvkm/mtklfb, so this only
# does what "pvrsrvctl --start --no-module" does: call SrvInit() from the
# libsrv_init.so blob.
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := pvrsrvinit
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := pvrsrvinit.c
LOCAL_SHARED_LIBRARIES := libdl liblog
LOCAL_CFLAGS := -Wall -Werror
include $(BUILD_EXECUTABLE)
