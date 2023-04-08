// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Hi3798MV200 Clock and Reset Generator Driver
 *
 * Copyright (c) 2016 HiSilicon Technologies Co., Ltd.
 */

#include <dt-bindings/clock/histb-clock.h>
#include <linux/clk-provider.h>
#include <dm/device.h>
#include <dm/lists.h>
#include <dm/read.h>

#include "clk.h"

/* hi3798MV200 core CRG */
#define HI3798MV200_INNER_CLK_OFFSET		64
#define HI3798MV200_FIXED_24M			65
#define HI3798MV200_FIXED_25M			66
#define HI3798MV200_FIXED_50M			67
#define HI3798MV200_FIXED_75M			68
#define HI3798MV200_FIXED_100M			69
#define HI3798MV200_FIXED_150M			70
#define HI3798MV200_FIXED_200M			71
#define HI3798MV200_FIXED_250M			72
#define HI3798MV200_FIXED_300M			73
#define HI3798MV200_FIXED_400M			74
#define HI3798MV200_MMC_MUX			75
#define HI3798MV200_ETH_PUB_CLK			76
#define HI3798MV200_ETH_BUS_CLK			77
#define HI3798MV200_ETH_BUS0_CLK		78
#define HI3798MV200_ETH_BUS1_CLK		79
#define HI3798MV200_COMBPHY1_MUX		80
#define HI3798MV200_FIXED_12M			81
#define HI3798MV200_FIXED_48M			82
#define HI3798MV200_FIXED_60M			83
#define HI3798MV200_FIXED_166P5M		84
#define HI3798MV200_SDIO0_MUX			85
#define HI3798MV200_COMBPHY0_MUX		86
#define HI3798MV200_SDIO1_MUX			87

static bool fixed_rate_registered;
static const struct hisi_fixed_rate_clock hi3798mv200_fixed_rate_clks[] = {
	{ HISTB_OSC_CLK, "clk_osc", 24000000, },
	{ HISTB_APB_CLK, "clk_apb", 100000000, },
	{ HISTB_AHB_CLK, "clk_ahb", 200000000, },
	{ HI3798MV200_FIXED_12M, "12m", 12000000, },
	{ HI3798MV200_FIXED_24M, "24m", 24000000, },
	{ HI3798MV200_FIXED_25M, "25m", 25000000, },
	{ HI3798MV200_FIXED_48M, "48m", 48000000, },
	{ HI3798MV200_FIXED_50M, "50m", 50000000, },
	{ HI3798MV200_FIXED_60M, "60m", 60000000, },
	{ HI3798MV200_FIXED_75M, "75m", 75000000, },
	{ HI3798MV200_FIXED_100M, "100m", 100000000, },
	{ HI3798MV200_FIXED_150M, "150m", 150000000, },
	{ HI3798MV200_FIXED_166P5M, "166p5m", 165000000, },
	{ HI3798MV200_FIXED_200M, "200m", 200000000, },
	{ HI3798MV200_FIXED_250M, "250m", 250000000, },
};

static const char *const mmc_mux_p[] = {
		"100m", "50m", "25m", "200m", "150m" };
static u32 mmc_mux_table[] = {0, 1, 2, 3, 6};

static const char *const comphy_mux_p[] = {
		"25m", "100m"};
static u32 comphy_mux_table[] = {0, 1};

static const char *const sdio_mux_p[] = {
		"100m", "50m", "150m", "166p5m" };
static u32 sdio_mux_table[] = {0, 1, 2, 3};

static struct hisi_mux_clock hi3798mv200_mux_clks[] = {
	{ HI3798MV200_MMC_MUX, "mmc_mux", mmc_mux_p, ARRAY_SIZE(mmc_mux_p),
		CLK_SET_RATE_PARENT, 0xa0, 8, 3, 0, mmc_mux_table, },
	{ HI3798MV200_COMBPHY0_MUX, "combphy0_mux",
		comphy_mux_p, ARRAY_SIZE(comphy_mux_p),
		CLK_SET_RATE_PARENT, 0x188, 3, 1, 0, comphy_mux_table, },
	{ HI3798MV200_SDIO0_MUX, "sdio0_mux", sdio_mux_p,
		ARRAY_SIZE(sdio_mux_p), CLK_SET_RATE_PARENT,
		0x9c, 8, 2, 0, sdio_mux_table, },
	{ HI3798MV200_SDIO1_MUX, "sdio1_mux", sdio_mux_p,
		ARRAY_SIZE(sdio_mux_p), CLK_SET_RATE_PARENT,
		0x28c, 8, 2, 0, sdio_mux_table, },
};

static const struct hisi_gate_clock hi3798mv200_gate_clks[] = {
	/* SDIO0 */
	{ HISTB_SDIO0_BIU_CLK, "clk_sdio0_biu", "200m",
			CLK_SET_RATE_PARENT, 0x9c, 0, 0, },
	{ HISTB_SDIO0_CIU_CLK, "clk_sdio0_ciu", "sdio0_mux",
		CLK_SET_RATE_PARENT, 0x9c, 1, 0, },
	/* EMMC */
	{ HISTB_MMC_BIU_CLK, "clk_mmc_biu", "200m",
		CLK_SET_RATE_PARENT, 0xa0, 0, 0, },
	{ HISTB_MMC_CIU_CLK, "clk_mmc_ciu", "mmc_mux",
		CLK_SET_RATE_PARENT, 0xa0, 1, 0, },
	/* Ethernet */
	{ HI3798MV200_ETH_PUB_CLK, "clk_pub", NULL,
		CLK_SET_RATE_PARENT, 0xcc, 5, 0, },
	{ HI3798MV200_ETH_BUS_CLK, "clk_bus", "clk_pub",
		CLK_SET_RATE_PARENT, 0xcc, 0, 0, },
	{ HI3798MV200_ETH_BUS0_CLK, "clk_bus_m0", "clk_bus",
		CLK_SET_RATE_PARENT, 0xcc, 1, 0, },
	{ HI3798MV200_ETH_BUS1_CLK, "clk_bus_m1", "clk_bus",
		CLK_SET_RATE_PARENT, 0xcc, 2, 0, },
	{ HISTB_ETH0_MAC_CLK, "clk_mac0", "clk_bus_m0",
		CLK_SET_RATE_PARENT, 0xcc, 3, 0, },
	{ HISTB_ETH0_MACIF_CLK, "clk_macif0", "clk_bus_m0",
		CLK_SET_RATE_PARENT, 0xcc, 24, 0, },
	{ HISTB_ETH1_MAC_CLK, "clk_mac1", "clk_bus_m1",
		CLK_SET_RATE_PARENT, 0xcc, 4, 0, },
	{ HISTB_ETH1_MACIF_CLK, "clk_macif1", "clk_bus_m1",
		CLK_SET_RATE_PARENT, 0xcc, 25, 0, },
	/* COMBPHY */
	{ HISTB_COMBPHY0_CLK, "clk_combphy0", "combphy0_mux",
		CLK_SET_RATE_PARENT, 0x188, 0, 0, },
	/* USB2 */
	{ HISTB_USB2_BUS_CLK, "clk_u2_bus", "clk_ahb",
		CLK_SET_RATE_PARENT, 0xb8, 0, 0, },
	{ HISTB_USB2_PHY_CLK, "clk_u2_phy", "60m",
		CLK_SET_RATE_PARENT, 0xb8, 4, 0, },
	{ HISTB_USB2_12M_CLK, "clk_u2_12m", "12m",
		CLK_SET_RATE_PARENT, 0xb8, 2, 0 },
	{ HISTB_USB2_48M_CLK, "clk_u2_48m", "48m",
		CLK_SET_RATE_PARENT, 0xb8, 1, 0 },
	{ HISTB_USB2_UTMI_CLK, "clk_u2_utmi", "60m",
		CLK_SET_RATE_PARENT, 0xb8, 5, 0 },
	{ HISTB_USB2_OTG_UTMI_CLK, "clk_u2_otg_utmi", "60m",
		CLK_SET_RATE_PARENT, 0xb8, 3, 0 },
	{ HISTB_USB2_PHY1_REF_CLK, "clk_u2_phy1_ref", "24m",
		CLK_SET_RATE_PARENT, 0xbc, 0, 0 },
	{ HISTB_USB2_PHY2_REF_CLK, "clk_u2_phy2_ref", "24m",
		CLK_SET_RATE_PARENT, 0xbc, 2, 0 },
	/* USB3 */
	{ HISTB_USB3_BUS_CLK, "clk_u3_bus", NULL,
		CLK_SET_RATE_PARENT, 0xb0, 0, 0 },
	{ HISTB_USB3_UTMI_CLK, "clk_u3_utmi", NULL,
		CLK_SET_RATE_PARENT, 0xb0, 4, 0 },
	{ HISTB_USB3_PIPE_CLK, "clk_u3_pipe", NULL,
		CLK_SET_RATE_PARENT, 0xb0, 3, 0 },
	{ HISTB_USB3_SUSPEND_CLK, "clk_u3_suspend", NULL,
		CLK_SET_RATE_PARENT, 0xb0, 2, 0 },
	{ HISTB_USB3_BUS_CLK1, "clk_u3_bus1", NULL,
		CLK_SET_RATE_PARENT, 0xb0, 16, 0 },
	{ HISTB_USB3_UTMI_CLK1, "clk_u3_utmi1", NULL,
		CLK_SET_RATE_PARENT, 0xb0, 20, 0 },
	{ HISTB_USB3_PIPE_CLK1, "clk_u3_pipe1", NULL,
		CLK_SET_RATE_PARENT, 0xb0, 19, 0 },
	{ HISTB_USB3_SUSPEND_CLK1, "clk_u3_suspend1", NULL,
		CLK_SET_RATE_PARENT, 0xb0, 18, 0 },
	/* SDIO1 */
	{ HISTB_SDIO1_BIU_CLK, "clk_sdio1_biu", "200m",
			CLK_SET_RATE_PARENT, 0x28c, 0, 0, },
	{ HISTB_SDIO1_CIU_CLK, "clk_sdio1_ciu", "sdio1_mux",
		CLK_SET_RATE_PARENT, 0x28c, 1, 0, },
};

static int hi3798mv200_clk_init(struct udevice *pdev)
{
	struct hisi_clock_data *data = dev_get_priv(pdev);
	return hisi_clk_init(pdev, data);
}

static int hi3798mv200_clk_register(struct udevice *pdev)
{
	struct hisi_clock_data *clk_data = dev_get_priv(pdev);
	int ret;

	if (!fixed_rate_registered)
	{
		ret = hisi_clk_register_fixed_rate(NULL, hi3798mv200_fixed_rate_clks,
					     ARRAY_SIZE(hi3798mv200_fixed_rate_clks));
		if (ret)
			goto done;
		fixed_rate_registered = true;
	}

	ret = hisi_clk_register_mux(NULL, hi3798mv200_mux_clks,
				ARRAY_SIZE(hi3798mv200_mux_clks),
				clk_data);
	if (ret)
		goto done;

	ret = hisi_clk_register_gate(NULL, hi3798mv200_gate_clks,
				ARRAY_SIZE(hi3798mv200_gate_clks),
				clk_data);
done:
	return ret;
}

static int hi3798mv200_clk_bind(struct udevice *pdev)
{
	ofnode node = dev_ofnode(pdev);
	return device_bind_driver_to_node(pdev, "hisilicon_reset", "hisilicon_reset", node, NULL);
};

static int hi3798mv200_clk_probe(struct udevice *pdev)
{
	return hi3798mv200_clk_register(pdev);
}

static const struct udevice_id hi3798mv200_crg_compat[] = {
	{ .compatible = "hisilicon,hi3798mv200-crg", },
	{ }
};

U_BOOT_DRIVER(hi3798mv200_crg) = {
	.name		= "hi3798mv200_crg",
	.id		= UCLASS_CLK,
	.bind		= hi3798mv200_clk_bind,
	.of_to_plat	= hi3798mv200_clk_init,
	.priv_auto	= sizeof(struct hisi_clock_data),
	.probe		= hi3798mv200_clk_probe,
	.of_match	= hi3798mv200_crg_compat,
	.ops		= &ccf_clk_ops,
};

/* hi3798MV200 sysctrl CRG */

static const struct hisi_gate_clock hi3798mv200_sysctrl_gate_clks[] = {
	{ HISTB_IR_CLK, "clk_ir", "24m",
		CLK_SET_RATE_PARENT, 0x48, 4, 0, },
	{ HISTB_TIMER01_CLK, "clk_timer01", "24m",
		CLK_SET_RATE_PARENT, 0x48, 6, 0, },
	{ HISTB_UART0_CLK, "clk_uart0", "75m",
		CLK_SET_RATE_PARENT, 0x48, 12, 0, },
};

static int hi3798mv200_sysctrl_clk_register(struct udevice *pdev)
{
	struct hisi_clock_data *clk_data = dev_get_priv(pdev);
	int ret;

	if (!fixed_rate_registered)
	{
		ret = hisi_clk_register_fixed_rate(NULL, hi3798mv200_fixed_rate_clks,
					     ARRAY_SIZE(hi3798mv200_fixed_rate_clks));
		if (ret)
			goto done;
		fixed_rate_registered = true;
	}

	ret = hisi_clk_register_gate(NULL, hi3798mv200_sysctrl_gate_clks,
				ARRAY_SIZE(hi3798mv200_sysctrl_gate_clks),
				clk_data);
done:
	return ret;
}

static int hi3798mv200_sysctrl_clk_probe(struct udevice *pdev)
{
	return hi3798mv200_sysctrl_clk_register(pdev);
}

static const struct udevice_id hi3798mv200_sysctrl_clk_match_table[] = {
	{ .compatible = "hisilicon,hi3798mv200-sysctrl", },
	{ }
};

U_BOOT_DRIVER(hi3798mv200_sysctrl) = {
	.name		= "hi3798mv200_sysctrl_clk",
	.id		= UCLASS_CLK,
	.of_to_plat	= hi3798mv200_clk_init,
	.priv_auto	= sizeof(struct hisi_clock_data),
	.probe		= hi3798mv200_sysctrl_clk_probe,
	.of_match	= hi3798mv200_sysctrl_clk_match_table,
	.ops		= &ccf_clk_ops,
};
