// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2025
 */

#include <fdtdec.h>
#include <linux/errno.h>

int meson_get_boot_device(void)
{
	return -ENOSYS;
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}
