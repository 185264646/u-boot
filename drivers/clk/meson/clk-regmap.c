// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2025
 */

#include <dm.h>
#include <linux/clk-provider.h>

#include "clk-regmap.h"

struct clk *meson_clk_register(struct udevice *dev, void __iomem *base, struct clk_common_data *data)
{
	union {
		struct clk_regmap_fixed_factor_data *fixed_factor_data;
		struct clk_regmap_mux_data *mux_data;
		struct clk_regmap_div_data *div_data;
		struct clk_regmap_gate_data *gate_data;
		void *data;
	} clk_data;

	if (!data)
		return NULL;

	clk_data.data = data->data;

	switch (data->type) {
		case CLK_UNKNOWN:
			assert_noisy(1);
			fallthrough;
		case CLK_FIXED_RATE:
			/* TODO */
			return NULL;

		case CLK_FIXED_FACTOR:
			return clk_register_fixed_factor(dev,
							 data->name,
							 data->parent_name,
							 data->flags,
							 clk_data.fixed_factor_data->mult,
							 clk_data.fixed_factor_data->div);

		case CLK_MUX:
			if (clk_data.mux_data->table)
				return clk_register_mux_table(dev,
							      data->name,
							      data->parent_names,
							      data->num_parents,
							      data->flags,
							      base + clk_data.mux_data->offset,
							      clk_data.mux_data->shift,
							      clk_data.mux_data->mask,
							      clk_data.mux_data->flags,
							      clk_data.mux_data->table);

			return clk_register_mux(dev,
						data->name,
						data->parent_names,
						data->num_parents,
						data->flags,
						base + clk_data.mux_data->offset,
						clk_data.mux_data->shift,
						fls(clk_data.mux_data->mask),
						clk_data.mux_data->flags);

		case CLK_DIVIDER:
			return clk_register_divider(dev,
						    data->name,
						    data->parent_name,
						    data->flags,
						    base + clk_data.div_data->offset,
						    clk_data.div_data->shift, clk_data.div_data->width,
						    clk_data.div_data->flags);

		case CLK_GATE:
			return clk_register_gate(dev,
						 data->name,
						 data->parent_name,
						 data->flags,
						 base + clk_data.gate_data->offset,
						 clk_data.gate_data->bit_idx,
						 clk_data.gate_data->flags,
						 NULL);

		default:
			//dev_err(dev, "Unknown clk_type %d\n", data->type);
	}

	return NULL;
}
