#
# Copyright (C) 2013 The Android Open-Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# can this be done in bash? credits go to bgcngm
define make_header
  perl -e 'print pack("a4 L a32 a472", "\x88\x16\x88\x58", $$ARGV[0], $$ARGV[1], "\xFF"x472)' $(1) $(2) > $(3)
endef

# assert-max-image-size uses the NAND formula (page + spare, minus a yaffs reserve),
# which is wrong for eMMC. Check the raw byte size instead.
# $(1): image, $(2): partition size in bytes
define assert-raw-image-size
  size=$$(stat -c %s $(1)); \
	if [ $$size -gt $(2) ]; then \
	  echo "error: $(1) too large ($$size > $(2))"; rm -f $(1); exit 1; \
	fi; \
	echo "$(1): $$size of $(2) bytes"
endef

# this is overriding targets from build/core/Makefile
$(INSTALLED_KERNEL_TARGET).mtk.header: $(INSTALLED_KERNEL_TARGET)
	size=$$($(call get-file-size,$(INSTALLED_KERNEL_TARGET))); \
		$(call make_header, $$((size)), "KERNEL", $@)
$(INSTALLED_KERNEL_TARGET).mtk: $(INSTALLED_KERNEL_TARGET).mtk.header
	$(call pretty,"Adding MTK header to kernel.")
	cat $(INSTALLED_KERNEL_TARGET).mtk.header $(INSTALLED_KERNEL_TARGET) \
		> $@

$(INSTALLED_RAMDISK_TARGET).mtk.header: $(INSTALLED_RAMDISK_TARGET)
	size=$$($(call get-file-size,$(INSTALLED_RAMDISK_TARGET))); \
		$(call make_header, $$((size)), "ROOTFS", $@)
$(INSTALLED_RAMDISK_TARGET).mtk: $(INSTALLED_RAMDISK_TARGET).mtk.header
	$(call pretty,"Adding MTK header to ramdisk.")
	cat $(INSTALLED_RAMDISK_TARGET).mtk.header $(INSTALLED_RAMDISK_TARGET) \
		> $@

# build/core/Makefile sets recovery_kernel with a trailing comment, so its value
# ends in a space and get-file-size (which quotes the path) would return nothing.
mtk_recovery_kernel := $(strip $(recovery_kernel))
$(PRODUCT_OUT)/recovery_kernel.mtk.header: $(mtk_recovery_kernel)
	size=$$($(call get-file-size,$(mtk_recovery_kernel))); \
		$(call make_header, $$((size)), "KERNEL", $@)
$(PRODUCT_OUT)/recovery_kernel.mtk: $(PRODUCT_OUT)/recovery_kernel.mtk.header
	$(call pretty,"Adding MTK header to recovery kernel.")
	cat $(PRODUCT_OUT)/recovery_kernel.mtk.header $(mtk_recovery_kernel) > $@

# The gzip recovery ramdisk makes recovery.img too big for the 6 MiB partition.
# The prebuilt kernel has CONFIG_RD_LZMA (its initramfs decompressors are gzip,
# bzip2 and lzma), so the recovery ramdisk is recompressed with LZMA.
$(recovery_ramdisk).lzma: $(recovery_ramdisk)
	$(call pretty,"Recompressing recovery ramdisk with LZMA.")
	gzip -dc $(recovery_ramdisk) | xz --format=lzma -9e -c > $@

$(recovery_ramdisk).mtk.header: $(recovery_ramdisk).lzma
	size=$$($(call get-file-size,$(recovery_ramdisk).lzma)); \
		$(call make_header, $$((size)), "RECOVERY", $@)
$(recovery_ramdisk).mtk:  $(MKBOOTIMG) $(recovery_ramdisk).mtk.header
	$(call pretty,"Adding MTK header to recovery ramdisk.")
	cat $(recovery_ramdisk).mtk.header $(recovery_ramdisk).lzma > $@

INTERNAL_MTK_BOOTIMAGE_ARGS := \
		--kernel $(INSTALLED_KERNEL_TARGET).mtk \
		--ramdisk $(INSTALLED_RAMDISK_TARGET).mtk

$(INSTALLED_BOOTIMAGE_TARGET): $(MKBOOTIMG)\
		$(INSTALLED_RAMDISK_TARGET).mtk $(INSTALLED_KERNEL_TARGET).mtk
	$(call pretty,"Target boot image: $@")
	$(MKBOOTIMG) $(INTERNAL_MTK_BOOTIMAGE_ARGS) $(BOARD_MKBOOTIMG_ARGS) \
		--output $@
	$(hide) $(call assert-raw-image-size,$@,$(BOARD_BOOTIMAGE_PARTITION_SIZE))
	@echo -e ${CL_CYN}"Made boot image: $@"${CL_RST}

INTERNAL_MTK_RECOVERYIMAGE_ARGS := \
		--kernel $(PRODUCT_OUT)/recovery_kernel.mtk \
		--ramdisk $(recovery_ramdisk).mtk

$(INSTALLED_RECOVERYIMAGE_TARGET): $(MKBOOTIMG) \
		$(recovery_ramdisk).mtk $(PRODUCT_OUT)/recovery_kernel.mtk
	@echo -e ${CL_CYN}"----- Making recovery image ------"${CL_RST}
	$(MKBOOTIMG) $(INTERNAL_MTK_RECOVERYIMAGE_ARGS) \
		$(BOARD_MKBOOTIMG_ARGS) --output $@
	$(hide) $(call assert-raw-image-size,$@,$(BOARD_RECOVERYIMAGE_PARTITION_SIZE))
	@echo -e ${CL_CYN}"Made recovery image: $@"${CL_RST}
