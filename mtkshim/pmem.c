/*
 * KK libcutils pmem API (MediaTek pmem-dev, cm-11.0 system/core faca7e01 +
 * 8000884b), on /dev/pmem_multimedia. Used by libJpgEncPipe.so and
 * libdpframework_os.so. The kernel has CONFIG_ANDROID_PMEM and registers
 * pmem_multimedia in mt_devs.c.
 *
 * Same behaviour as the MTK version, minus its pm.dumpstack debug traces.
 */
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <android/log.h>

#include "android_pmem.h"

#define LOG_TAG "utils_pmem"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define PMEM_DEVICE_NAME "/dev/pmem_multimedia"

static size_t align_size(size_t size)
{
    return (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

static void *do_alloc(size_t size, int *pfd)
{
    struct pmem_region region = { 0, 0 };
    size_t aligned_size;
    void *base;
    int fd;

    if (!pfd)
        return NULL;
    *pfd = -1;

    fd = open(PMEM_DEVICE_NAME, O_RDWR | O_SYNC);
    if (fd < 0) {
        LOGE("[PMEM] open %s failed: %s", PMEM_DEVICE_NAME, strerror(errno));
        return NULL;
    }
    aligned_size = align_size(size);
    base = mmap(NULL, aligned_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (base == MAP_FAILED) {
        LOGE("[PMEM] mmap size %zu failed: %s", aligned_size, strerror(errno));
        close(fd);
        return NULL;
    }
    region.len = aligned_size;
    if (ioctl(fd, PMEM_MAP, &region) < 0) {
        LOGE("[PMEM] PMEM_MAP size %zu failed: %s", aligned_size, strerror(errno));
        munmap(base, aligned_size);
        close(fd);
        return NULL;
    }
    *pfd = fd;
    return base;
}

/* The MTK pmem_alloc and pmem_alloc_sync were identical (both O_SYNC). */
void *pmem_alloc(size_t size, int *pfd)
{
    return do_alloc(size, pfd);
}

void *pmem_alloc_sync(size_t size, int *pfd)
{
    return do_alloc(size, pfd);
}

int pmem_free(void *ptr, size_t size, int fd)
{
    size_t aligned_size = align_size(size);
    struct pmem_region region = { 0, aligned_size };
    int ret = 0;

    if (ioctl(fd, PMEM_UNMAP, &region) < 0) {
        LOGE("PMEM_UNMAP size %zu failed!", size);
        ret = -1;
    }
    if (munmap(ptr, aligned_size) < 0) {
        LOGE("munmap size %zu failed!", size);
        ret = -1;
    }
    if (close(fd) < 0) {
        LOGE("Close file %d failed!", fd);
        ret = -1;
    }
    return ret;
}

void *pmem_get_phys(int fd)
{
    struct pmem_region region = { 0, 0 };

    if (ioctl(fd, PMEM_GET_PHYS, &region) < 0) {
        LOGE("PMEM_GET_PHYS failed: 0x%X !", fd);
        return NULL;
    }
    return (void *)region.offset;
}

void pmem_cache_flush(int fd, unsigned int offset, unsigned int length)
{
    struct pmem_region region = { offset, length };

    if (fd < 0 || offset == 0 || length == 0) {
        LOGE("pmem_cache_flush: invalid argument");
        return;
    }
    if (ioctl(fd, PMEM_CACHE_FLUSH, &region) < 0)
        LOGE("PMEM_CACHE_FLUSH offset 0x%08x, size %u failed!", offset, length);
}
