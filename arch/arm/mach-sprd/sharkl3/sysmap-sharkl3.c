// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * SpreadTrum UNISOC Sharkl3 memory map
 *
 * Copyright 2023 (C) Yang Xiwen <forbidden405@foxmail.com>
 */

#include <asm/armv8/mmu.h>

static struct mm_region sharkl3_memmap[] = {
	{
		.virt = 0x0UL, /* Peripheral block */
		.phys = 0x0UL, /* Peripheral block */
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x80000000UL, /* DRAM */
		.phys = 0x80000000UL, /* DRAM */
		.size = 0x200000000UL, /* 8G at most */
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List Terminator */
		0,
	}
};

struct mm_region *mem_map = sharkl3_memmap;

