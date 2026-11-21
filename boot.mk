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

# The stock gzip recovery ramdisk (3.4 MB) doesn't fit the 6 MiB partition next to
# the 3.2 MB kernel, so the recovery root is repacked as legacy .lzma (about 1.7 MB).
# Our kernel has CONFIG_RD_LZMA; the old TWRP 2.7.1 prebuilt kernel did not (it
# panicked in mount_block_root), so don't use this with that kernel.
# For initramfs, unlzma allocates a buffer of the header's dictionary size (a
# streamed .lzma has no output size), so the dictionary is capped at 8 MiB, which
# still covers the whole uncompressed cpio (~5.2 MB). -9 would mean 64 MiB.
# Files listed in RECOVERY_RAMDISK_TRIM are left out (none needed right now).
RECOVERY_RAMDISK_TRIM :=
RECOVERY_RAMDISK_LZMA_FLAGS := --format=lzma --lzma1=preset=9e,dict=8MiB
recovery_trim_root := $(PRODUCT_OUT)/recovery-trimmed-root

$(recovery_ramdisk).lzma: $(recovery_ramdisk) $(MKBOOTFS)
	$(call pretty,"Repacking recovery ramdisk as lzma.")
	rm -rf $(recovery_trim_root)
	cp -a $(TARGET_RECOVERY_ROOT_OUT) $(recovery_trim_root)
	$(if $(strip $(RECOVERY_RAMDISK_TRIM)),rm -f $(addprefix $(recovery_trim_root)/,$(RECOVERY_RAMDISK_TRIM)))
	$(MKBOOTFS) $(recovery_trim_root) | xz $(RECOVERY_RAMDISK_LZMA_FLAGS) > $@

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
