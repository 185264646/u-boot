/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 BayLibre, SAS.
 * Author: Jerome Brunet <jbrunet@baylibre.com>
 */

#ifndef __MESON_CLK_MPLL_H
#define __MESON_CLK_MPLL_H

#include <linux/clk-provider.h>
#include <linux/compat.h>

#include "parm.h"

/* FIXME: move to include/regmap.h */
struct reg_sequence {
	unsigned int reg;
	unsigned int def;
	unsigned int delay_us;
};

struct meson_clk_mpll_data {
	struct parm sdm;
	struct parm sdm_en;
	struct parm n2;
	struct parm ssen;
	struct parm misc;
	const struct reg_sequence *init_regs;
	unsigned int init_count;
	spinlock_t *lock;
	u8 flags;
};

#define CLK_MESON_MPLL_ROUND_CLOSEST	BIT(0)
#define CLK_MESON_MPLL_SPREAD_SPECTRUM	BIT(1)

extern const struct clk_ops meson_clk_mpll_ro_ops;
extern const struct clk_ops meson_clk_mpll_ops;

#ifdef CONFIG_CLK_MESON_MPLL
struct clk *clk_register_meson_mpll(struct udevice *dev, const char *name,
				    const char *parent_name, unsigned long flags,
				    void *base, const struct meson_clk_mpll_data *data,
				    bool readonly);
#else
static inline struct clk *clk_register_meson_mpll(struct udevice *dev, const char *name,
						  const char *parent_name, unsigned long flags,
						  void *base, const struct meson_clk_mpll_data *data,
						  bool readonly)
{
	return NULL;
}
#endif

#endif /* __MESON_CLK_MPLL_H */
