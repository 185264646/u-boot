// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * IOCONFIG pinctrl driver for HiSTB SoCs
 *
 * Copyright 2024 (r) Yang Xiwen <forbidden405@outlook.com>
 */

#include <asm/io.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/pinctrl.h>
#include <dm/read.h>
#include <linux/bitfield.h>
#include <linux/kernel.h>
#include <linux/types.h>

#include "pinctrl-histb.h"

static int histb_pinctrl_get_pins_count(struct udevice *dev)
{
	struct histb_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->pin_nums;
}

static const char *histb_pinctrl_get_pin_name(struct udevice *dev, unsigned int selector)
{
	struct histb_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->pins[selector].name;
}

static int histb_pinctrl_get_functions_count(struct udevice *dev)
{
	struct histb_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->func_nums;
}

static const char *histb_pinctrl_get_function_name(struct udevice *dev, unsigned int selector)
{
	struct histb_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->funcs[selector].name;
}

static int histb_pinctrl_pinmux_set(struct udevice *dev, unsigned int pin_selector,
				    unsigned int func_selector)
{
	struct histb_pinctrl_priv *priv = dev_get_priv(dev);
	const struct histb_pin_function_desc *func = &priv->funcs[func_selector];
	const struct histb_pin_func_setup_entry *func_setup_tbl = func->cfg_tbl;
	int i;
	u32 reg;
	void __iomem *pin_reg = (u32 *)priv->base + priv->pins[pin_selector].number;
	bool found = false;

	for (i = 0; i < func->pins_num; i++) {
		if (func_setup_tbl[i].pin_selector == pin_selector) {
			reg = readl(pin_reg);
			reg &= ~HISTB_PIN_FUNC_MASK;
			reg |= func_setup_tbl[i].mask;
			writel(reg, pin_reg);

			found = true;
			break;
		}
	}

	if (!found) {
		dev_err(dev, "Unable to set pin %d to the given function %d\n",
			pin_selector, func_selector);
		return -EINVAL;
	}

	return 0;
}

static const struct pinconf_param histb_pinctrl_pinconf_params[] = {
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pulldown", PIN_CONFIG_BIAS_PULL_DOWN, 0 },
	{ "bias-pullup", PIN_CONFIG_BIAS_PULL_UP, 0 },
	{ "input-schmitt-enable", PIN_CONFIG_INPUT_SCHMITT_ENABLE, 1 },
	{ "input-schmitt-disable", PIN_CONFIG_INPUT_SCHMITT_ENABLE, 0 },
	{ "drive-strength", PIN_CONFIG_DRIVE_STRENGTH, 1 },
	{ "slew-rate", PIN_CONFIG_SLEW_RATE, 1 },
};

enum histb_pinctrl_bias_status {
	BIAS_PULL_DOWN,
	BIAS_PULL_UP,
	BIAS_DISABLE,
};

static int histb_pinctrl_set_bias(struct udevice *dev, unsigned int selector,
				  enum histb_pinctrl_bias_status status)
{
	struct histb_pinctrl_priv *priv = dev_get_priv(dev);
	void __iomem *pin_reg = (u32 *)priv->base + priv->pins[selector].number;
	u32 reg = readl(pin_reg);

	reg &= ~(HISTB_PIN_PULLDOWN | HISTB_PIN_PULLUP);

	switch (status) {
	case BIAS_DISABLE:
		break;
	case BIAS_PULL_DOWN:
		reg |= HISTB_PIN_PULLDOWN;
		break;
	case BIAS_PULL_UP:
		reg |= HISTB_PIN_PULLUP;
		break;
	}

	writel(reg, pin_reg);

	return 0;
}

static int histb_pinctrl_set_slew_rate(struct udevice *dev, unsigned int pin_selector,
				       unsigned int argument)
{
	struct histb_pinctrl_priv *priv = dev_get_priv(dev);
	void __iomem *pin_reg = (u32 *)priv->base + priv->pins[pin_selector].number;
	u32 reg = readl(pin_reg);

	if (likely(argument == 1)) {
		reg |= HISTB_PIN_SLEWRATE;
	} else if (argument == 0) {
		reg &= ~HISTB_PIN_SLEWRATE;
	} else {
		dev_err(dev, "slew rate argument can be only 0 or 1!\n");
		return -EINVAL;
	}

	writel(reg, pin_reg);

	return 0;
}

static int histb_pinctrl_set_schmitt(struct udevice *dev, unsigned int pin_selector,
				     unsigned int argument)
{
	struct histb_pinctrl_priv *priv = dev_get_priv(dev);
	void __iomem *pin_reg = (u32 *)priv->base + priv->pins[pin_selector].number;
	u32 reg = readl(pin_reg);

	if (likely(argument == 0)) {
		reg &= ~HISTB_PIN_SCHMITT;
	} else if (argument == 1) {
		reg |= HISTB_PIN_SCHMITT;
	} else {
		dev_err(dev, "schmitt argument can be only 0 or 1!\n");
		return -EINVAL;
	}

	writel(reg, pin_reg);

	return 0;
}

static int histb_pinctrl_set_drive_strength(struct udevice *dev, unsigned int pin_selector,
					    unsigned int argument)
{
	struct histb_pinctrl_priv *priv = dev_get_priv(dev);
	const u8 *drv_tbl = priv->pins[pin_selector].drv_tbl;
	void __iomem *pin_reg = (u32 *)priv->base + priv->pins[pin_selector].number;
	u32 reg = readl(pin_reg);
	int i = -1;

	if (unlikely(argument == 0)) {
		dev_err(dev, "the minimal drive strength is 1mA!\n");
		return -EINVAL;
	}

	if (unlikely(!drv_tbl)) {
		dev_err(dev, "pin %u does not support setting drive strength!\n", pin_selector);
		return -ENOENT;
	}

	// calculate the largest drive-strength that does not exceeds the given value
	// if the lowest value is still too large, use that anyway
	// TODO: use bsearch()?
	while (drv_tbl[++i] > argument)
		;

	if (!drv_tbl[i])
		i--;

	reg &= ~HISTB_PIN_DRV_MASK;
	reg |= FIELD_PREP(HISTB_PIN_DRV_MASK, i);

	debug("%s: setting drive strength of pin %s to %d\n", __func__, priv->pins[pin_selector].name, drv_tbl[i]);
	writel(reg, pin_reg);

	return 0;
}

static int histb_pinctrl_pinconf_set(struct udevice *dev, unsigned int pin_selector,
				     unsigned int param, unsigned int argument)
{
	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		return histb_pinctrl_set_bias(dev, pin_selector, BIAS_DISABLE);
	case PIN_CONFIG_BIAS_PULL_UP:
		return histb_pinctrl_set_bias(dev, pin_selector, BIAS_PULL_UP);
	case PIN_CONFIG_BIAS_PULL_DOWN:
		return histb_pinctrl_set_bias(dev, pin_selector, BIAS_PULL_DOWN);
	case PIN_CONFIG_SLEW_RATE:
		return histb_pinctrl_set_slew_rate(dev, pin_selector, argument);
	case PIN_CONFIG_INPUT_SCHMITT_ENABLE:
		return histb_pinctrl_set_schmitt(dev, pin_selector, argument);
	case PIN_CONFIG_DRIVE_STRENGTH:
		return histb_pinctrl_set_drive_strength(dev, pin_selector, argument);
	}

	dev_err(dev, "can't handle given config %d\n", param);
	return -EINVAL;
}

static int histb_pinctrl_get_pin_muxing(struct udevice *dev, unsigned int selector,
					char *buf, int size)
{
	struct histb_pinctrl_priv *priv = dev_get_priv(dev);
	const struct histb_pin_mux_desc *desc = priv->pins[selector].func_tbl;
	u32 reg = readl((u32 *)priv->base + priv->pins[selector].number);
	bool found = false;
	int current;

	current = FIELD_GET(HISTB_PIN_FUNC_MASK, reg);

	while (desc->name) {
		if (desc->bits == current) {
			strlcpy(buf, desc->name, size);
			found = true;
			break;
		}
		desc++;
	}

	if (!found) {
		dev_warn(dev, "unknown pinmux selected\n");
		strcpy(buf, "UNKNOWN!");
	}

	return 0;
}

const struct pinctrl_ops histb_pinctrl_ops = {
	.get_pins_count = histb_pinctrl_get_pins_count,
	.get_pin_name = histb_pinctrl_get_pin_name,
	.get_functions_count = histb_pinctrl_get_functions_count,
	.get_function_name = histb_pinctrl_get_function_name,
	.pinmux_set = histb_pinctrl_pinmux_set,
	.pinconf_num_params = ARRAY_SIZE(histb_pinctrl_pinconf_params),
	.pinconf_params = histb_pinctrl_pinconf_params,
	.pinconf_set = histb_pinctrl_pinconf_set,
	.set_state = pinctrl_generic_set_state,
	.get_pin_muxing = histb_pinctrl_get_pin_muxing,
};

int histb_pinctrl_of_to_plat(struct udevice *dev)
{
	struct histb_pinctrl_priv *priv = (void *)dev_get_driver_data(dev);

	priv->base = dev_remap_addr(dev);
	if (!priv->base) {
		dev_err(dev, "failed to remap addr\n");
		return -EINVAL;
	}

	dev_set_priv(dev, priv);

	return 0;
}
