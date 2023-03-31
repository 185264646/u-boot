/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2023 Yang Xiwen <forbidden405@outlook.com>
 *
 * Configurations for Henan Guangdian HC2910. Derived from other ARM devices.
 */

#ifndef _HC2910_2AGHD05_CONFIG_H_
#define _HC2910_2AGHD05_CONFIG_H_

#define FDTFILE "hisilicon/" CONFIG_DEFAULT_DEVICE_TREE ".dtb"

/* Note that the BootROM loads U-Boot at 0xC40000, at least don't touch 0xC00000 - 0xC40000 because BL31 residents there */

#define MEM_LAYOUT_ENV_SETTINGS \
	"scriptaddr=0xe00000\0" \
	"pxefile_addr_r=0xf00000\0" \
	"loadaddr=0x1000000\0" \
	"kernel_addr_r=${loadaddr}\0" \
	"fdt_addr_r=0x3000000\0" \
	"fdtfile=" FDTFILE "\0" \
	"ramdisk_addr_r=0x3100000\0"

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)

#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS \
	MEM_LAYOUT_ENV_SETTINGS \
	BOOTENV

#endif
