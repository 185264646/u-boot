// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025 Yang Xiwen <forbidden405@outlook.com>
 */

#include <dm/device.h>
#include <regmap.h>
#include <wdt.h>

#define WATCHDOG_TC	0x00
#define WATCHDOG_RESET	0x04

struct meson_wdt_priv {
	struct regmap *regmap;
	struct meson_wdt_data *data;
};

struct meson_wdt_data {
	unsigned int enable;
	unsigned int terminal_count_mask;
	unsigned int count_unit;
	unsigned int max_timeout_ms;
};

static struct meson_wdt_data meson6_wdt_data = {
	.enable			= BIT(22),
	.terminal_count_mask	= 0x3fffff,
	.count_unit		= 100000, /* 10 us */
	.max_timeout_ms		= 41943
};

static struct meson_wdt_data meson8b_wdt_data = {
	.enable			= BIT(19),
	.terminal_count_mask	= 0xffff,
	.count_unit		= 7812, /* 128 us */
	.max_timeout_ms		= 8388
};

static int meson_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct meson_wdt_priv *priv = dev_get_priv(dev);
	u32 val;

	if (timeout_ms > priv->data->max_timeout_ms)
		return -ERANGE;

	val = (u64) timeout_ms * priv->data->count_unit / 1000;
	val &= priv->data->terminal_count_mask;

	/* disable wdt */
	regmap_update_bits(priv->regmap, WATCHDOG_TC, priv->data->enable, 0);

	/* reset counter */
	regmap_write(priv->regmap, WATCHDOG_RESET, 0);

	/* enable wdt, set timeout */
	regmap_update_bits(priv->regmap, WATCHDOG_TC,
			   priv->data->enable | priv->data->terminal_count_mask,
			   priv->data->enable | val);

	return 0;
}

static int meson_wdt_stop(struct udevice *dev)
{
	struct meson_wdt_priv *priv = dev_get_priv(dev);

	return regmap_update_bits(priv->regmap, WATCHDOG_TC, priv->data->enable, 0);
}

static int meson_wdt_reset(struct udevice *dev)
{
	struct meson_wdt_priv *priv = dev_get_priv(dev);

	return regmap_write(priv->regmap, WATCHDOG_RESET, 0);
}

static const struct wdt_ops meson_wdt_ops = {
	.start	= meson_wdt_start,
	.stop	= meson_wdt_stop,
	.reset	= meson_wdt_reset
};

static int meson_wdt_probe(struct udevice *dev)
{
	struct meson_wdt_priv *priv = dev_get_priv(dev);
	int ret;

	ret = regmap_init_mem(dev_ofnode(dev), &priv->regmap);
	if (ret)
		return ret;

	priv->data = (void *)dev_get_driver_data(dev);

	/* Reset the watchdog to initial state */
	regmap_write(priv->regmap, WATCHDOG_TC, 0);
	regmap_write(priv->regmap, WATCHDOG_RESET, 0);

	return 0;
}

static const struct udevice_id meson_wdt_dt_ids[] = {
		{ .compatible = "amlogic,meson6-wdt", .data = (ulong) &meson6_wdt_data },
		{ .compatible = "amlogic,meson8-wdt", .data = (ulong) &meson6_wdt_data },
		{ .compatible = "amlogic,meson8b-wdt", .data = (ulong) &meson8b_wdt_data },
		{ .compatible = "amlogic,meson8m2-wdt", .data = (ulong) &meson8b_wdt_data },
		{ /* sentinel */ }
};

U_BOOT_DRIVER(meson_mx_wdt) = {
	.name		= "meson_mx_wdt",
	.id		= UCLASS_WDT,
	.of_match	= meson_wdt_dt_ids,
	.priv_auto	= sizeof(struct meson_wdt_priv),
	.probe		= meson_wdt_probe,
	.ops		= &meson_wdt_ops,
	.flags		= DM_FLAG_PRE_RELOC
};
