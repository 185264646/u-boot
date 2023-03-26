// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Hisilicon clock driver
 * Adapted from linux kernel
 */

#include <dm/read.h>
#include <linux/clk-provider.h>
#include "clk.h"

spinlock_t lock;

int hisi_clk_init(struct udevice *dev, struct hisi_clock_data *data)
{
	data->base = dev_remap_addr(dev);
	if(!data->base)
		return -EINVAL;

	return 0;
}

int hisi_clk_register_fixed_rate(struct device *dev, const struct hisi_fixed_rate_clock *clks,
					 int nums)
{
	struct clk *clk;
	int i;

	for (i = 0; i < nums; i++) {
		clk = clk_register_fixed_rate(dev, clks[i].name,
					      clks[i].fixed_rate);
		if (IS_ERR(clk)) {
			pr_err("%s: failed to register clock %s\n",
			       __func__, clks[i].name);
			goto err;
		}
		clk_dm(clks[i].id, clk);
	}
	return 0;

err:
	return PTR_ERR(clk);
}

int hisi_clk_register_mux(struct device *dev,
				  const struct hisi_mux_clock *clks,
				  int nums,
				  const struct hisi_clock_data *data)
{
	struct clk *clk;
	void __iomem *base = data->base;
	int i;

	for (i = 0; i < nums; i++) {
		u32 mask = BIT(clks[i].width) - 1;

		clk = clk_register_mux_table(dev, clks[i].name,
					clks[i].parent_names,
					clks[i].num_parents, clks[i].flags,
					base + clks[i].offset, clks[i].shift,
					mask, clks[i].mux_flags, clks[i].table);
		if (IS_ERR(clk)) {
			pr_err("%s: failed to register clock %s\n",
			       __func__, clks[i].name);
			goto err;
		}

		clk_dm(clks[i].id, clk);
	}
	return 0;

err:
	return PTR_ERR(clk);
}

int hisi_clk_register_gate(struct device *dev, const struct hisi_gate_clock *clks,
				       int nums, const struct hisi_clock_data *data)
{
	struct clk *clk;
	void __iomem *base = data->base;
	int i;

	for (i = 0; i < nums; i++) {
		clk = clk_register_gate(dev, clks[i].name,
						clks[i].parent_name,
						clks[i].flags,
						base + clks[i].offset,
						clks[i].bit_idx,
						clks[i].gate_flags,
						&lock);
		if (IS_ERR(clk)) {
			pr_err("%s: failed to register clock %s\n",
			       __func__, clks[i].name);
			goto err;
		}

		clk_dm(clks[i].id, clk);
	}
	return 0;

err:
	return PTR_ERR(clk);
}
