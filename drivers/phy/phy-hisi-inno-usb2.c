// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * INNO USB2 PHY driver for HiSilicon HiSTB SoCs
 *
 * Copyright (r) 2024 Yang Xiwen <forbidden405@outlook.com>
 */

#include <clk.h>
#include <generic-phy.h>
#include <reset.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/read.h>

/*
 * Currently there are 2 types of bus found being used for this PHY
 *
 * - One is plain MMIO bus, found on Hi3798MV2x
 * - The other is some unknown local bus controlled by peri module, found on Hi3798CV200, Hi3798MV100 etc..
 *
 * Only MMIO bus is supported so far.
 *
 * FIXME: convert to regmap API and implement a bus driver when full regmap support is available.
 */

#define PHY_CLK_ENABLE	BIT(2)

struct hisi_inno_phy_port_priv {
	void __iomem *port_base;
	struct reset_ctl *utmi_rst;
};

struct hisi_inno_phy_priv {
	void __iomem *phy_base;
	struct reset_ctl_bulk *resets;
	struct clk *ref_clk;
	struct hisi_inno_phy_port *ports;
};

static int hisi_inno_phy_port_of_to_plat(struct udevice *dev)
{
	struct hisi_inno_phy_port_priv *priv = dev_get_priv(dev);

	priv->port_base = dev_remap_addr(dev);
	if (!priv->port_base) {
		dev_err(dev, "failed to remap addr\n");
		return -EINVAL;
	}

	priv->utmi_rst = devm_reset_control_get(dev, NULL);
	if (IS_ERR(priv->utmi_rst)) {
		dev_err(dev, "failed to get utmi reset %ld\n", PTR_ERR(priv->utmi_rst));
		return PTR_ERR(priv->utmi_rst);
	}

	return 0;
}

static int hisi_inno_phy_port_init(struct phy *phy)
{
	struct hisi_inno_phy_port_priv *priv = dev_get_priv(phy->dev);
	u8 reg;

	reset_deassert(priv->utmi_rst);
	udelay(20);

	/* The phy clk is controlled by the port0 register 0x06. */
	/* Note MMIO bus is mapped in a special way, so 0x18 is correct */
	reg = readb(priv->port_base + 0x18);
	reg |= PHY_CLK_ENABLE;
	writeb(reg, priv->port_base + 0x18);

	return 0;
}

struct phy_ops hisi_inno_phy_port_ops = {
	.init = hisi_inno_phy_port_init,
};

U_BOOT_DRIVER(phy_hisi_inno_phy_port) = {
	.name		= "phy-hisi-inno-phy-usb2-port",
	.id		= UCLASS_PHY,
	.of_to_plat	= hisi_inno_phy_port_of_to_plat,
	.ops		= &hisi_inno_phy_port_ops,
	.priv_auto	= sizeof(struct hisi_inno_phy_port_priv),
};

int hisi_inno_phy_of_to_plat(struct udevice *dev)
{
	struct hisi_inno_phy_priv *priv = dev_get_priv(dev);

	priv->phy_base = dev_remap_addr(dev);
	if (!priv->phy_base) {
		dev_err(dev, "failed to remap addr\n");
		return -EINVAL;
	}

	priv->resets = devm_reset_bulk_get(dev);
	if (IS_ERR(priv->resets)) {
		dev_err(dev, "failed to get resets %ld\n", PTR_ERR(priv->resets));
		return PTR_ERR(priv->resets);
	}

	priv->ref_clk = devm_clk_get(dev, NULL);
	if (IS_ERR(priv->ref_clk)) {
		dev_err(dev, "failed to get ref clk %ld\n", PTR_ERR(priv->ref_clk));
		return PTR_ERR(priv->ref_clk);
	}

	return 0;
}

int hisi_inno_phy_probe(struct udevice *dev)
{
	struct hisi_inno_phy_priv *priv = dev_get_priv(dev);

	clk_enable(priv->ref_clk);
	reset_deassert_bulk(priv->resets);

	return 0;
}

int hisi_inno_phy_bind(struct udevice *dev)
{
	ofnode subnode;
	int ret;

	ofnode_for_each_subnode(subnode, dev_ofnode(dev)) {
		ret = device_bind_driver_to_node(dev, "phy-hisi-inno-phy-usb2-port",
						 ofnode_get_name(subnode), subnode, NULL);
		if (ret) {
			dev_err(dev, "error binding node %s: %d\n", ofnode_get_name(subnode), ret);
			return ret;
		}
	};

	return 0;
}


struct udevice_id hisi_inno_phy_ids[] = {
	{ .compatible = "hisilicon,hi3798mv200-usb2-phy", },
	{ },
};

U_BOOT_DRIVER(phy_hisi_inno_usb2) = {
	.name		= "phy-hisi-inno-usb2",
	.id		= UCLASS_SIMPLE_BUS,
	.bind		= hisi_inno_phy_bind,
	.of_to_plat	= hisi_inno_phy_of_to_plat,
	.probe		= hisi_inno_phy_probe,
	.of_match	= hisi_inno_phy_ids,
	.priv_auto	= sizeof(struct hisi_inno_phy_priv),
};
