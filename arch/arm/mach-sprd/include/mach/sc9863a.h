// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Register definitions for SC9863A
 */

#ifndef __SC9863A_H__
#define __SC9863A_H__

#include <linux/types.h>

#define GICD_BASE		(0x01800000)
/* AHB_EB */
struct reg_ahb_eb {
	u32 :4;
	u32 otg_eb :1;
	u32 dma_eb :1;
	u32 nandc_eb :1;
	u32 sdio0_eb :1;
	u32 sdio1_eb :1;
	u32 sdio2_eb :1;
	u32 emmc_eb :1;
	u32 :15;
	u32 emmc_32k_eb :1;
	u32 sdio0_32k_eb :1;
	u32 sdio1_32k_eb :1;
	u32 sdio2_32k_eb :1;
	u32 nandc_26m_eb :1;
};

#endif //  __SC9863A_H__
