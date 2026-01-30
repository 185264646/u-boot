// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 BayLibre, SAS
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <phy.h>
#include "designware.h"
#include <dm/device_compat.h>
#include <linux/err.h>

#define ETH_REG_0		0x0
#define ETH_REG_1		0x4
#define ETH_REG_2		0x18
#define ETH_REG_3		0x1c

#define GX_ETH_REG_0_PHY_INTF		BIT(0)
/* mux to choose between fclk_div2 (bit unset) and mpll2 (bit set) */
#define GX_ETH_REG_0_CLK_M250_SEL_MASK	GENMASK(4, 4)
#define GX_ETH_REG_0_TX_PHASE(x)	(((x) & 3) << 5)
#define GX_ETH_REG_0_TX_RATIO(x)	(((x) & 7) << 7)
#define GX_ETH_REG_0_PHY_CLK_EN	BIT(10)
#define GX_ETH_REG_0_INVERT_RMII_CLK	BIT(11)
#define GX_ETH_REG_0_CLK_EN		BIT(12)
/* Bypass (= 0, the signal from the GPIO input directly connects to the
 * internal sampling) or enable (= 1) the internal logic for RXEN and RXD[3:0]
 * timing tuning.
 */
#define PRG_ETH0_ADJ_ENABLE		BIT(13)
/* Controls whether the RXEN and RXD[3:0] signals should be aligned with the
 * input RX rising/falling edge and sent to the Ethernet internals. This sets
 * the automatically delay and skew automatically (internally).
 */
#define PRG_ETH0_ADJ_SETUP		BIT(14)

#define AXG_ETH_REG_0_PHY_INTF_RGMII	BIT(0)
#define AXG_ETH_REG_0_PHY_INTF_RMII	BIT(2)
#define AXG_ETH_REG_0_TX_PHASE(x)	(((x) & 3) << 5)
#define AXG_ETH_REG_0_TX_RATIO(x)	(((x) & 7) << 7)
#define AXG_ETH_REG_0_PHY_CLK_EN	BIT(10)
#define AXG_ETH_REG_0_INVERT_RMII_CLK	BIT(11)
#define AXG_ETH_REG_0_CLK_EN		BIT(12)

struct dwmac_meson8b_plat {
	struct dw_eth_pdata dw_eth_pdata;
	int (*dwmac_setup)(struct udevice *dev, struct eth_pdata *edata);
	void *regs;
};

static int dwmac_meson8b_of_to_plat(struct udevice *dev)
{
	struct dwmac_meson8b_plat *pdata = dev_get_plat(dev);

	pdata->regs = dev_read_addr_index_ptr(dev, 1);
	if (!pdata->regs)
		return -EINVAL;

	pdata->dwmac_setup = (void *)dev_get_driver_data(dev);
	if (!pdata->dwmac_setup)
		return -EINVAL;

	return designware_eth_of_to_plat(dev);
}

static int dwmac_setup_axg(struct udevice *dev, struct eth_pdata *edata)
{
	struct dwmac_meson8b_plat *plat = dev_get_plat(dev);

	switch (edata->phy_interface) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
		/* Set RGMII mode */
		setbits_le32(plat->regs + ETH_REG_0, AXG_ETH_REG_0_PHY_INTF_RGMII |
						     AXG_ETH_REG_0_TX_PHASE(1) |
						     AXG_ETH_REG_0_TX_RATIO(4) |
						     AXG_ETH_REG_0_PHY_CLK_EN |
						     AXG_ETH_REG_0_CLK_EN);
		break;

	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		/* TOFIX: handle amlogic,tx-delay-ns & rx-internal-delay-ps from DT */
		setbits_le32(plat->regs + ETH_REG_0, AXG_ETH_REG_0_PHY_INTF_RGMII |
						     AXG_ETH_REG_0_TX_RATIO(4) |
						     AXG_ETH_REG_0_PHY_CLK_EN |
						     AXG_ETH_REG_0_CLK_EN);
		break;

	case PHY_INTERFACE_MODE_RMII:
		/* Set RMII mode */
		out_le32(plat->regs + ETH_REG_0, AXG_ETH_REG_0_PHY_INTF_RMII |
						 AXG_ETH_REG_0_INVERT_RMII_CLK |
						 AXG_ETH_REG_0_CLK_EN);
		break;
	default:
		dev_err(dev, "Unsupported PHY mode\n");
		return -EINVAL;
	}

	return 0;
}

static int dwmac_setup_gx(struct udevice *dev, struct eth_pdata *edata)
{
	struct dwmac_meson8b_plat *plat = dev_get_plat(dev);

	switch (edata->phy_interface) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
		/* Set RGMII mode */
		setbits_le32(plat->regs + ETH_REG_0, GX_ETH_REG_0_PHY_INTF |
						     GX_ETH_REG_0_TX_PHASE(1) |
						     GX_ETH_REG_0_TX_RATIO(4) |
						     GX_ETH_REG_0_PHY_CLK_EN |
						     GX_ETH_REG_0_CLK_EN);

		break;

	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		/* TOFIX: handle amlogic,tx-delay-ns & rx-internal-delay-ps from DT */
		setbits_le32(plat->regs + ETH_REG_0, GX_ETH_REG_0_PHY_INTF |
						     GX_ETH_REG_0_TX_RATIO(4) |
						     GX_ETH_REG_0_PHY_CLK_EN |
						     GX_ETH_REG_0_CLK_EN);

		break;

	case PHY_INTERFACE_MODE_RMII:
		/* Set RMII mode */
		out_le32(plat->regs + ETH_REG_0, GX_ETH_REG_0_INVERT_RMII_CLK |
						 GX_ETH_REG_0_CLK_EN);

		if (!IS_ENABLED(CONFIG_MESON_GXBB))
			writel(0x10110181, plat->regs + ETH_REG_2);

		break;
	default:
		dev_err(dev, "Unsupported PHY mode\n");
		return -EINVAL;
	}

	return 0;
}

static bool is_phy_interface_mode_rgmii(phy_interface_t mode)
{
	return mode == PHY_INTERFACE_MODE_RGMII
		|| mode == PHY_INTERFACE_MODE_RGMII_ID
		|| mode == PHY_INTERFACE_MODE_RGMII_RXID
		|| mode == PHY_INTERFACE_MODE_RGMII_TXID;
}

static int dwmac_setup_mx(struct udevice *dev, struct eth_pdata *edata)
{
	struct dwmac_meson8b_plat *plat = dev_get_plat(dev);
	struct clk *rgmii_tx_clk = NULL;
	struct clk *clk;
	int ret;
	u32 val = GX_ETH_REG_0_PHY_INTF | GX_ETH_REG_0_CLK_M250_SEL_MASK |
		GX_ETH_REG_0_TX_RATIO(2) | GX_ETH_REG_0_PHY_CLK_EN | GX_ETH_REG_0_CLK_EN;

	if (is_phy_interface_mode_rgmii(edata->phy_interface)) {
		/* Always use clkin1 to provide 125MHz RGMII tx clock */
		rgmii_tx_clk = devm_clk_get_optional(dev, "clkin1");
		if (IS_ERR(rgmii_tx_clk)) {
			log_err("Failed to get clock \"clkin1\": %ld\n", PTR_ERR(rgmii_tx_clk));
			return PTR_ERR(rgmii_tx_clk);
		}
	}

	/* It is divided by 4 internally to provide rgmii tx clock */
	ret = clk_set_rate(rgmii_tx_clk, 125000000 * 4);
	if (IS_ERR_VALUE(ret)) {
		log_err("Failed to set clock rate for RGMII tx clock: %d\n", ret);
		return ret;
	}

	ret = clk_enable(rgmii_tx_clk);
	if (ret) {
		return ret;
	}

	switch (edata->phy_interface) {
	case PHY_INTERFACE_MODE_RGMII_RXID:
		/* TODO: defaults to 2ns */
		val |= GX_ETH_REG_0_TX_PHASE(1);
		fallthrough;
	case PHY_INTERFACE_MODE_RGMII_ID:
		/* Set RGMII mode */
		writel(val, plat->regs + ETH_REG_0);
		break;

	case PHY_INTERFACE_MODE_RGMII:
		val |= GX_ETH_REG_0_TX_PHASE(1);
		fallthrough;
	case PHY_INTERFACE_MODE_RGMII_TXID:
		clk = devm_clk_get(dev, "timing-adjustment");
		if (IS_ERR(clk)) {
			log_err("Failed to get \"timing-adjustment\" clock: %ld\n", PTR_ERR(clk));
			return PTR_ERR(clk);
		}

		ret = clk_enable(clk);
		if (ret) {
			log_err("Failed to enable \"timing-adjustment\" clock: %d\n", ret);
			return ret;
		}

		val |= PRG_ETH0_ADJ_SETUP | PRG_ETH0_ADJ_ENABLE;
		writel(val, plat->regs + ETH_REG_0);

		break;

	case PHY_INTERFACE_MODE_RMII:
		/* Set RMII mode */
		clrsetbits_le32(plat->regs + ETH_REG_0, GX_ETH_REG_0_MASK,
				GX_ETH_REG_0_INVERT_RMII_CLK |
				GX_ETH_REG_0_CLK_EN);

		break;

	default:
		dev_err(dev, "Unsupported PHY mode\n");
		return -EINVAL;
	}

	return 0;
}

static int dwmac_meson8b_probe(struct udevice *dev)
{
	struct dwmac_meson8b_plat *pdata = dev_get_plat(dev);
	struct eth_pdata *edata = &pdata->dw_eth_pdata.eth_pdata;
	int ret;

	ret = pdata->dwmac_setup(dev, edata);
	if (ret)
		return ret;

	return designware_eth_probe(dev);
}

static const struct udevice_id dwmac_meson8b_ids[] = {
	{ .compatible = "amlogic,meson8b-dwmac", .data = (ulong)dwmac_setup_mx },
	{ .compatible = "amlogic,meson-gxbb-dwmac", .data = (ulong)dwmac_setup_gx },
	{ .compatible = "amlogic,meson-g12a-dwmac", .data = (ulong)dwmac_setup_axg },
	{ .compatible = "amlogic,meson-axg-dwmac", .data = (ulong)dwmac_setup_axg },
	{ }
};

U_BOOT_DRIVER(dwmac_meson8b) = {
	.name		= "dwmac_meson8b",
	.id		= UCLASS_ETH,
	.of_match	= dwmac_meson8b_ids,
	.of_to_plat = dwmac_meson8b_of_to_plat,
	.probe		= dwmac_meson8b_probe,
	.ops		= &designware_eth_ops,
	.priv_auto	= sizeof(struct dw_eth_dev),
	.plat_auto	= sizeof(struct dwmac_meson8b_plat),
	.flags		= DM_FLAG_ALLOC_PRIV_DMA,
};
