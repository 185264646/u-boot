// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2016 AmLogic, Inc.
 * Author: Michael Turquette <mturquette@baylibre.com>
 */

/*
 * MultiPhase Locked Loops are outputs from a PLL with additional frequency
 * scaling capabilities. MPLL rates are calculated as:
 *
 * f(N2_integer, SDM_IN ) = 2.0G/(N2_integer + SDM_IN/16384)
 */

#include <div64.h>
#include <linux/clk-provider.h>
#include <linux/compat.h>
#include <dm/device.h>

#include "clk-mpll.h"

#define SDM_DEN 16384
#define N2_MIN	4
#define N2_MAX	511

#define CLK_MPLL_DRV_NAME	"clk_meson_mpll"
#define CLK_MPLL_RO_DRV_NAME	"clk_meson_mpll_ro"

#define to_clk_mpll(_clk)	container_of(_clk, struct clk_mpll, clk)

struct clk_mpll {
	struct clk clk;
	const struct meson_clk_mpll_data *data;
	void *reg;
};

static long rate_from_params(unsigned long parent_rate,
			     unsigned int sdm,
			     unsigned int n2)
{
	unsigned long divisor = (SDM_DEN * n2) + sdm;

	if (n2 < N2_MIN)
		return -EINVAL;

	return DIV_ROUND_UP_ULL((u64)parent_rate * SDM_DEN, divisor);
}

static void params_from_rate(unsigned long requested_rate,
			     unsigned long parent_rate,
			     unsigned int *sdm,
			     unsigned int *n2,
			     u8 flags)
{
	uint64_t div = parent_rate;
	uint64_t frac = do_div(div, requested_rate);

	frac *= SDM_DEN;

	if (flags & CLK_MESON_MPLL_ROUND_CLOSEST)
		*sdm = DIV_ROUND_CLOSEST_ULL(frac, requested_rate);
	else
		*sdm = DIV_ROUND_UP_ULL(frac, requested_rate);

	if (*sdm == SDM_DEN) {
		*sdm = 0;
		div += 1;
	}

	if (div < N2_MIN) {
		*n2 = N2_MIN;
		*sdm = 0;
	} else if (div > N2_MAX) {
		*n2 = N2_MAX;
		*sdm = SDM_DEN - 1;
	} else {
		*n2 = div;
	}
}

static unsigned long mpll_recalc_rate(struct clk *clk)
{
	struct clk_mpll *mpll = to_clk_mpll(clk);
	unsigned int sdm, n2;
	ulong parent_rate;
	long rate;

	parent_rate = clk_get_parent_rate(clk);
	if (IS_ERR_VALUE(parent_rate))
		return parent_rate;

	sdm = meson_parm_read(mpll->reg, &mpll->data->sdm);
	n2 = meson_parm_read(mpll->reg, &mpll->data->n2);

	rate = rate_from_params(parent_rate, sdm, n2);
	return rate < 0 ? 0 : rate;
}

static ulong mpll_determine_rate(struct clk *clk, ulong rate_req)
{
	struct clk_mpll *mpll = to_clk_mpll(clk);
	unsigned int sdm, n2;
	ulong parent_rate;
	ulong rate;

	parent_rate = clk_get_parent_rate(clk);
	if (IS_ERR_VALUE(parent_rate))
		return parent_rate;

	params_from_rate(rate_req, parent_rate, &sdm, &n2,
			 mpll->data->flags);

	rate = rate_from_params(parent_rate, sdm, n2);

	return rate;
}

static ulong mpll_set_rate(struct clk *clk, unsigned long rate)
{
	struct clk_mpll *mpll = to_clk_mpll(clk);
	unsigned int sdm, n2;
	unsigned long flags = 0;
	ulong parent_rate;

	parent_rate = clk_get_parent_rate(clk);
	if (IS_ERR_VALUE(parent_rate))
		return parent_rate;

	params_from_rate(rate, parent_rate, &sdm, &n2, mpll->data->flags);

	if (mpll->data->lock)
		spin_lock_irqsave(mpll->lock, flags);
	else
		__acquire(mpll->lock);

	/* Set the fractional part */
	meson_parm_write(mpll->reg, &mpll->data->sdm, sdm);

	/* Set the integer divider part */
	meson_parm_write(mpll->reg, &mpll->data->n2, n2);

	if (mpll->data->lock)
		spin_unlock_irqrestore(mpll->lock, flags);
	else
		__release(mpll->lock);

	return rate;
}

static int mpll_init(struct clk *clk)
{
	struct clk_mpll *mpll = to_clk_mpll(clk);

#ifndef __UBOOT__
	/* TBD */
	if (mpll->init_count)
		regmap_multi_reg_write(clk->map, mpll->init_regs,
				       mpll->init_count);
#endif

	/* Enable the fractional part */
	meson_parm_write(mpll->reg, &mpll->data->sdm_en, 1);

	/* Set spread spectrum if possible */
	if (MESON_PARM_APPLICABLE(&mpll->data->ssen)) {
		unsigned int ss =
			mpll->data->flags & CLK_MESON_MPLL_SPREAD_SPECTRUM ? 1 : 0;
		meson_parm_write(mpll->reg, &mpll->data->ssen, ss);
	}

	/* Set the magic misc bit if required */
	if (MESON_PARM_APPLICABLE(&mpll->data->misc))
		meson_parm_write(mpll->reg, &mpll->data->misc, 1);

	return 0;
}

const struct clk_ops meson_clk_mpll_ro_ops = {
	.get_rate	= mpll_recalc_rate,
	.round_rate	= mpll_determine_rate,
};

U_BOOT_DRIVER(clk_mpll_ro) = {
	.name	= CLK_MPLL_RO_DRV_NAME,
	.id	= UCLASS_CLK,
	.ops	= &meson_clk_mpll_ro_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

const struct clk_ops meson_clk_mpll_ops = {
	.get_rate	= mpll_recalc_rate,
	.round_rate	= mpll_determine_rate,
	.set_rate	= mpll_set_rate,
	.enable		= mpll_init,
};

U_BOOT_DRIVER(clk_mpll) = {
	.name	= CLK_MPLL_DRV_NAME,
	.id	= UCLASS_CLK,
	.ops	= &meson_clk_mpll_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

struct clk *clk_register_meson_mpll(struct udevice *dev, const char *name,
				    const char *parent_name, unsigned long flags,
				    void *base, const struct meson_clk_mpll_data *data, 
				    bool readonly)
{
	struct clk_mpll *mpll;
	int ret;

	mpll = kzalloc(sizeof(*mpll), GFP_KERNEL);
	if (!mpll)
		return ERR_PTR(-ENOMEM);

	mpll->reg = base;
	mpll->data = data;
	mpll->clk.flags = flags;

	ret = clk_register(&mpll->clk, readonly ? CLK_MPLL_RO_DRV_NAME : CLK_MPLL_DRV_NAME,
			   name, parent_name);
	if (ret) {
		kfree(mpll);
		return ERR_PTR(ret);
	}

	return &mpll->clk;
}

MODULE_DESCRIPTION("Amlogic MPLL driver");
MODULE_AUTHOR("Michael Turquette <mturquette@baylibre.com>");
MODULE_LICENSE("GPL");
