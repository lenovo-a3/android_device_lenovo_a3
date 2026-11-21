#
# Shims for the MT6589 KitKat blobs running on Lollipop (cm-12.1).
#
# libmtkshim: symbols the blobs expect from KK framework libraries that L no
# longer has. cm-12.1 has no LD_SHIM_LIBS, so it is LD_PRELOADed from
# init.mt8389.rc (the L linker looks in LD_PRELOAD libs before a library's own
# DT_NEEDED). It links only libc/libdl/liblog, because it is loaded into every
# process; anything from other libraries is looked up with dlsym at call time.
#
# libcorkscrew: L dropped libcorkscrew, but gralloc.mt8389.so has it in
# DT_NEEDED, so a library of that name has to exist.
#
# Symbol list: see shim-audit.txt / tools/blob-audit.py in the workspace.
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libmtkshim
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := \
	audio.c \
	callstack.c \
	netutils.c \
	pmem.c \
	ssl.c \
	tracer.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_SHARED_LIBRARIES := liblog libdl
LOCAL_CFLAGS := -Wall -Werror
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcorkscrew
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := corkscrew.c
LOCAL_CFLAGS := -Wall -Werror
include $(BUILD_SHARED_LIBRARY)
