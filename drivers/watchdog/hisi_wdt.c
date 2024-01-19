// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Watchdog driver for HiSilicon SoCs
 *
 * Copyright 2024 (r) Yang Xiwen <forbidden405@outlook.com>
 */

#include <clk.h>
#include <errno.h>
#include <log.h>
#include <reset.h>
#include <wdt.h>
#include <dm/device_compat.h>
#include <dm/device.h>
#include <dm/lists.h>
#include <dm/read.h>
#include <linux/delay.h>
#include <linux/math64.h>
#include <linux/types.h>

/* CONTROL register definitions */
#define HISI_WDG_RES_EN	BIT(1)
#define HISI_WDG_INT_EN	BIT(0)

/* RIS(Raw Interrupt Status) register definitions */
#define HISI_WDG_RIS	BIT(0)

/* MIS(Masked Interrupt Status) register definitions*/
#define HISI_WDG_MIS	BIT(0)

/* LOCK register magic */
// Write this value to unlock watchdog
#define HISI_WDG_LOCK_MAGIC	0x1ACCE551
// Read values
#define HISI_WDG_LOCK_WA	0x0
#define HISI_WDG_LOCK_RO	0x1

struct hisi_wdg_reg {
	u32 load; // 0x0000
	u32 value; // 0x0004
	u32 control; // 0x0008
	u32 intclr; // 0x000c
	u32 ris; // 0x0010
	u32 mis; // 0x0014
	u32 reserved[762]; // 0x0018
	u32 lock; // 0x0c00
};

struct hisi_wdt_priv {
	struct hisi_wdg_reg __iomem *reg;
	struct clk *clk;
	struct reset_ctl *rst;
};

static inline void hisi_wdt_unlock(struct hisi_wdg_reg __iomem *reg)
{
	reg->lock = HISI_WDG_LOCK_MAGIC;
}

static inline void hisi_wdt_lock(struct hisi_wdg_reg __iomem *reg)
{
	// Any value other than HISI_WDG_LOCK_MAGIC would lock the registers
	reg->lock = 0;
}

static int hisi_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct hisi_wdt_priv *priv = dev_get_priv(dev);
	u64 rate;
	u64 val;

	rate = clk_get_rate(priv->clk);

	/* This may overflow */
	val = mul_u64_u32_div(timeout_ms, rate, 1000);
	if (val > UINT32_MAX) {
		dev_warn(dev, "timeout_ms too large, using maximum.\n");
		val = UINT32_MAX;
	}

	hisi_wdt_unlock(priv->reg);

	priv->reg->load = (u32) val;
	priv->reg->control |= (HISI_WDG_RES_EN | HISI_WDG_INT_EN);

	hisi_wdt_lock(priv->reg);

	return 0;
}

static int hisi_wdt_stop(struct udevice *dev)
{
	struct hisi_wdt_priv *priv = dev_get_priv(dev);

	hisi_wdt_unlock(priv->reg);
	// disabling interrupt also disables counting
	priv->reg->control &= ~HISI_WDG_INT_EN;

	hisi_wdt_lock(priv->reg);

	return 0;
}

static int hisi_wdt_reset(struct udevice *dev)
{
	struct hisi_wdt_priv *priv = dev_get_priv(dev);

	hisi_wdt_unlock(priv->reg);

	// any value written to INTCLR would result a counter reload
	priv->reg->intclr = 0;

	hisi_wdt_lock(priv->reg);

	return 0;
}

static int hisi_wdt_expire_now(struct udevice *dev, ulong flags)
{
	return hisi_wdt_start(dev, 1, flags);
}

static const struct wdt_ops hisi_wdt_ops = {
	.start		= hisi_wdt_start,
	.stop		= hisi_wdt_stop,
	.reset		= hisi_wdt_reset,
	.expire_now	= hisi_wdt_expire_now,
};

static int hisi_wdt_probe(struct udevice *dev)
{
	struct hisi_wdt_priv *priv = dev_get_priv(dev);
	int ret;

	ret = clk_prepare_enable(priv->clk);
	if (ret) {
		dev_err(dev, "failed to enable clk: %d\n", ret);
		return log_msg_ret("clk", ret);
	}

	ret = reset_assert(priv->rst);
	if (ret) {
		dev_err(dev, "failed to assert reset: %d\n", ret);
		return log_msg_ret("rst", ret);
	}

	udelay(10);

	ret = reset_deassert(priv->rst);
	if (ret) {
		dev_err(dev, "failed to deassert reset: %d\n", ret);
		return log_msg_ret("rst", ret);
	}

	return 0;
}

static int hisi_wdt_of_to_plat(struct udevice *dev)
{
	struct hisi_wdt_priv *priv = dev_get_priv(dev);

	priv->reg = dev_remap_addr(dev);
	if (!priv->reg) {
		dev_err(dev, "failed to remap\n");
		return log_msg_ret("wdt", -EINVAL);
	}

	priv->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(priv->clk)) {
		dev_err(dev, "failed to get clk: %ld\n", PTR_ERR(priv->clk));
		return log_msg_ret("wdt", PTR_ERR(priv->clk));
	}

	priv->rst = devm_reset_control_get(dev, NULL);
	if (IS_ERR(priv->rst)) {
		dev_err(dev, "failed to get rst: %ld\n", PTR_ERR(priv->rst));
		return log_msg_ret("wdt", PTR_ERR(priv->rst));
	}

	return 0;
}

static const struct udevice_id hisi_wdt_ids[] = {
	{ .compatible = "hisilicon,wdt" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(hisi_wdt) = {
	.name		= "hisilicon_wdt",
	.id		= UCLASS_WDT,
	.of_match	= hisi_wdt_ids,
	.of_to_plat	= hisi_wdt_of_to_plat,
	.probe		= hisi_wdt_probe,
	.priv_auto	= sizeof(struct hisi_wdt_priv),
	.ops		= &hisi_wdt_ops,
};
