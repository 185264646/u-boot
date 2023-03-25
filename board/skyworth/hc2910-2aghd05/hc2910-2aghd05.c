/*
 * Board init file for Skyworth HC2910 2AGHD05
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <init.h>
#include <asm/system.h>

int board_init(void)
{
	return 0;
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
}

void reset_cpu(void)
{
	psci_system_reset();
}
