// SPDX-License-Identifier: GPL-2.0+
/*
 * Board init file for Skyworth HC2910 2AGHD05
 */

#include <common.h>
#include <fdtdec.h>
#include <init.h>
#include <asm/system.h>
#include <linux/io.h>

#define HI3798MV200_PERI_CTRL_BASE 0xf8a20000
#define SDIO0_LDO_OFFSET	0x11c

// SDIO LDO bypassed, 3.3V
#define SDIO0_LDO_VAL	0x60

#define REG_IOCONFIG_BASE	0xf8a21000
#define IOSHARE(n)		(0x4 * (n))

// pull-up, slew-rate, drive-strength 8mA, function eMMC
// same for ccmd, clock and data pins
#define EMMC_COMMON_CONF	0x1192

// pull-up, slew-rate, drive-strength 12mA, function SDIO
#define SDIO_COMMON_CONF	0x1133

static void sdio0_set_ldo(void)
{
	writel(SDIO0_LDO_VAL, HI3798MV200_PERI_CTRL_BASE + SDIO0_LDO_OFFSET);
}

static void emmc_ioconfig(void)
{
	void __iomem *base = (void *) REG_IOCONFIG_BASE;
	static const u32 offsets[] = {
		IOSHARE(4), IOSHARE(5), IOSHARE(6), IOSHARE(7),
		IOSHARE(8), IOSHARE(9), IOSHARE(10), IOSHARE(11),
		IOSHARE(12), IOSHARE(14)
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(offsets); i++) {
		writel(EMMC_COMMON_CONF, base + offsets[i]);
	}
}

static void sdio_ioconfig(void)
{
	void __iomem *base = (void *) REG_IOCONFIG_BASE;
	static const u32 offsets[] = {
		IOSHARE(49), IOSHARE(50), IOSHARE(51),
		IOSHARE(52), IOSHARE(53), IOSHARE(54)
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(offsets); i++) {
		writel(SDIO_COMMON_CONF, base + offsets[i]);
	}
}

static int board_sdmmc_init(void)
{
	sdio0_set_ldo();
	emmc_ioconfig();
	sdio_ioconfig();

	return 0;
}

int board_init(void)
{
	board_sdmmc_init();
	return 0;
}
