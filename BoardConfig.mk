# Copyright (C) 2013 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := device/lenovo/a3

-include vendor/lenovo/a3/BoardConfigVendor.mk

# board
TARGET_BOOTLOADER_BOARD_NAME := a3000_prc_3gfull
TARGET_BOARD_PLATFORM := mt6589
TARGET_ARCH := arm
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_SMP := true
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_VARIANT := cortex-a7

TARGET_OTA_ASSERT_DEVICE := a3,a3000_prc_3gfull,A3000,a3000

# blob hacks
COMMON_GLOBAL_CFLAGS += -DMR1_AUDIO_BLOB -DDISABLE_HW_ID_MATCH_CHECK -DNEEDS_VECTORIMPL_SYMBOLS -DMTK_G_MT6589
COMMON_GLOBAL_CPPFLAGS += -DMTK_G_MT6589
# cm-12.1's own MTK support (liblog __xlog_buf_printf, stagefright buffer alignment, ...)
COMMON_GLOBAL_CFLAGS += -DMTK_HARDWARE
BOARD_HAS_MTK_HARDWARE := true
# libutils RefBase::moveReferences, called by hwcomposer.mt8389.so and libcam.*
COMMON_GLOBAL_CFLAGS += -DREFBASE_JB_MR1_COMPAT_SYMBOLS
BOARD_HAVE_PRE_KITKAT_AUDIO_BLOB := true
BOARD_HAVE_PRE_KITKAT_AUDIO_POLICY_BLOB := true
TARGET_RUNNING_WITHOUT_SYNC_FRAMEWORK := true

# BT
BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_MTK := true
BOARD_BLUETOOTH_DOES_NOT_USE_RFKILL := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(LOCAL_PATH)/bluetooth

# power
TARGET_POWERHAL_VARIANT := cm

# boot
TARGET_NO_BOOTLOADER := true
BOARD_CUSTOM_BOOTIMG_MK := $(LOCAL_PATH)/boot.mk
# Put our boot.img (with the MTK headers) into target-files BOOTABLE_IMAGES/,
# otherwise releasetools rebuilds it with plain mkbootimg and lk can't boot it
BOARD_CUSTOM_BOOTIMG := true

# EGL settings
USE_OPENGL_RENDERER := true
BOARD_EGL_WORKAROUND_BUG_10194508 := true

# kernel
# Built from kernel/lenovo/a3 (lenovo-a3/android_kernel_lenovo_mt8389, cm-12.1).
# a3_defconfig = the MTK-generated config (mt6589 + lenovo89_tb_twn_a_jb2 + a3000_*)
# plus LZMA kernel compression and KALLSYMS. The MTK kernel Makefile also needs
# TARGET_KERNEL_PRODUCT to find its custom/ and config/ dirs; the a3000_* 3G
# variants have identical kernel configs.
TARGET_KERNEL_SOURCE := kernel/lenovo/a3
TARGET_KERNEL_CONFIG := a3_defconfig
export TARGET_KERNEL_PRODUCT := a3000_row_call
# Fallback, used only while kernel/lenovo/a3 is absent: zImage from TWRP 2.7.1.0 FUSE,
# MTK header stripped (boot.mk adds it back).
TARGET_PREBUILT_KERNEL := $(LOCAL_PATH)/prebuilt/kernel

# boot.mk calls mkbootimg with BOARD_MKBOOTIMG_ARGS only, so base and pagesize go here too.
# The cmdline is left empty, like the stock and TWRP 2.7.1 images: init takes ro.hardware
# (mt8389, from "Hardware : MT8389") from /proc/cpuinfo.
BOARD_KERNEL_BASE := 0x10000000
BOARD_KERNEL_PAGESIZE := 2048
BOARD_MKBOOTIMG_ARGS := --base 0x10000000 --pagesize 2048 \
    --kernel_offset 0x00008000 --ramdisk_offset 0x01000000 \
    --second_offset 0x00f00000 --tags_offset 0x00000100

# partition info (FUSE scatter)
# BOOTIMG and RECOVERY are 6 MiB (scatter and /proc/dumchar_info agree).
BOARD_BOOTIMAGE_PARTITION_SIZE := 6291456
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 6291456
# ANDROID 0x10f80000..0x42f80000
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 838860800
# USRDATA runs to the end of the eMMC. TODO: set from dumchar_info (minus the 16 KiB crypto footer).
#BOARD_USERDATAIMAGE_PARTITION_SIZE :=
BOARD_FLASH_BLOCK_SIZE := 512
TARGET_USERIMAGES_USE_EXT4 := true

# vendor/cm sets ro.adb.secure=1. CM Recovery can't show an adb key prompt; it only
# trusts /data/misc/adb/adb_keys, which a fresh /data doesn't have. During bring-up,
# turn adb auth off on non-user builds. The first value in the list wins, so it goes first.
ifneq ($(TARGET_BUILD_VARIANT),user)
ADDITIONAL_DEFAULT_PROPERTIES := ro.adb.secure=0 $(ADDITIONAL_DEFAULT_PROPERTIES)
endif

# recovery (CM Recovery, FUSE layout)
TARGET_RECOVERY_FSTAB := $(LOCAL_PATH)/recovery.fstab
# No recovery-from-boot patch: imgdiff can't diff our LZMA recovery ramdisk
# against the gzip boot ramdisk, and the ROM shouldn't rewrite RECOVERY at boot.
BOARD_CANT_BUILD_RECOVERY_FROM_BOOT_PATCH := true
# ...and none in target-files either (see the script)
TARGET_RELEASETOOL_MAKE_RECOVERY_PATCH_SCRIPT := $(LOCAL_PATH)/releasetools/make_recovery_patch
# Untested on the MTK framebuffer: if colours are wrong, try "BGRA_8888" / "RGBX_8888";
# if the screen stays black or does not update, try BOARD_RECOVERY_NEEDS_FBIOPAN_DISPLAY := true.
#TARGET_RECOVERY_PIXEL_FORMAT := "BGRA_8888"

# wifi
WPA_SUPPLICANT_VERSION := VER_0_8_X
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_HOSTAPD_DRIVER := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_mtk
BOARD_HOSTAPD_PRIVATE_LIB   := lib_driver_cmd_mtk

# dummy FW paths to trigger libhardware_legacy/wifi FW switch through hald
WIFI_DRIVER_FW_PATH_AP := "AP"
WIFI_DRIVER_FW_PATH_STA := "STA"
WIFI_DRIVER_FW_PATH_P2P := "STA+P2P"

# telephony
BOARD_RIL_CLASS := ../../../$(LOCAL_PATH)/ril/

# nvram permission fix
TARGET_SPECIFIC_HEADER_PATH := $(LOCAL_PATH)/include
