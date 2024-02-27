// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Common functions for sharkl3
 *
 * Copyright 2023 Yang Xiwen <forbidden405@foxmail.com>
 */

#include <init.h>
#include <asm/system.h>

int __weak board_init(void)
{
	return 0;
}

void __weak reset_cpu(void)
{
	// The downstream U-Boot uses watchdog for resetting the system
	// PSCI functions are only used in Linux
	// This might be a bug in PSCI implementation that it fails when secondary cores are not up
	// We follow downstream u-boot instead
#if 0
	// Once the bug is fixed, we can get back to PSCI reset instead
	psci_system_reset();
#endif
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}
