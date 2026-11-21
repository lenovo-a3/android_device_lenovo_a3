/*
 * Userspace part of the kernel's include/linux/android_pmem.h
 * (kernel/lenovo/a3, Copyright (C) 2007 Google, Inc., GPLv2).
 * Bionic's L kernel headers no longer ship it.
 */
#ifndef _MTKSHIM_ANDROID_PMEM_H_
#define _MTKSHIM_ANDROID_PMEM_H_

#include <sys/ioctl.h>

#define PMEM_IOCTL_MAGIC 'p'
#define PMEM_GET_PHYS		_IOW(PMEM_IOCTL_MAGIC, 1, unsigned int)
#define PMEM_MAP		_IOW(PMEM_IOCTL_MAGIC, 2, unsigned int)
#define PMEM_GET_SIZE		_IOW(PMEM_IOCTL_MAGIC, 3, unsigned int)
#define PMEM_UNMAP		_IOW(PMEM_IOCTL_MAGIC, 4, unsigned int)
#define PMEM_ALLOCATE		_IOW(PMEM_IOCTL_MAGIC, 5, unsigned int)
#define PMEM_CONNECT		_IOW(PMEM_IOCTL_MAGIC, 6, unsigned int)
#define PMEM_GET_TOTAL_SIZE	_IOW(PMEM_IOCTL_MAGIC, 7, unsigned int)
#define PMEM_CACHE_FLUSH	_IOW(PMEM_IOCTL_MAGIC, 8, unsigned int)

struct pmem_region {
	unsigned long offset;
	unsigned long len;
};

#endif
