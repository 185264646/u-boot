// SPDX-License-Identifier: GPL-2.0-or-later
/* BOOT0 hook for Amlogic Meson MX SoCs */

#ifndef __BOOT0_H
#define __BOOT0_H

#if IS_ENABLED(CONFIG_MESON_MX)
/*
 * The SPL image from vendor checks offset 0x3c-0x3f and mandates them to be 0x12345678.
 */
_start:
	ARM_VECTORS

	.skip 0x3c - (. - _start)
	.word 0x12345678
#endif

#endif /* __BOOT0_H */
