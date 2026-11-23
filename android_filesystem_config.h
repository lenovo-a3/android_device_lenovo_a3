/*
 * Device overrides for fs_config (build/tools/fs_config, generated into
 * /system/etc/fs_config_files; read before libcutils' built-in table, both
 * by the image build and at runtime).
 *
 * surfaceflinger: N gives /system/bin/surfaceflinger the file capability
 * CAP_SYS_NICE (libcutils/fs_config.c). A file capability makes the kernel
 * set AT_SECURE, and bionic then drops LD_PRELOAD (libc_init_common.cpp),
 * so libmtkshim never loads into surfaceflinger and the GPU blobs fail to
 * link (libsrv_um.so: cannot locate symbol "__xlog_buf_printf"), and
 * surfaceflinger aborts with "couldn't find an OpenGL ES implementation".
 * Same mode/owner as N, no capabilities (as on M). surfaceflinger only
 * loses SCHED_FIFO for its main thread ("Couldn't set SCHED_FIFO").
 */

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <private/android_filesystem_config.h>

#define NO_ANDROID_FILESYSTEM_CONFIG_DEVICE_DIRS 1

static const struct fs_path_config android_device_files[] = {
    { 00755, AID_SYSTEM, AID_GRAPHICS, 0, "system/bin/surfaceflinger" },
#ifdef NO_ANDROID_FILESYSTEM_CONFIG_DEVICE_DIRS
    { 00000, AID_ROOT,   AID_ROOT,     0, "system/etc/fs_config_dirs" },
#endif
    { 00000, AID_ROOT,   AID_ROOT,     0, "system/etc/fs_config_files" },
};
