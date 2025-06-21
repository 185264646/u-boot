/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Ported from Linux kernel source.
 * Despite of the file name clk-regmap.h, this is unrelated to regmap
 *
 * Copyright (c) 2018 BayLibre, SAS.
 * Author: Jerome Brunet <jbrunet@baylibre.com>
 */

#ifndef __CLK_REGMAP_H
#define __CLK_REGMAP_H

#include <linux/clk-provider.h>

enum clk_type {
	CLK_UNKNOWN,
	CLK_FIXED_RATE,
	CLK_FIXED_FACTOR,
	CLK_MUX,
	CLK_DIVIDER,
	CLK_GATE,
	CLK_PLL,
	CLK_TYPE_NUM,
};

/**
 * struct clk_common_data - contains common data to register a CCF clock
 *
 * @clk_type:		type of the clock
 * @parent_name:	name of the parent clock
 * @parent_names:	names of the parents clocks
 * @num_parents:	number of parents(optional)
 * @flags:		clk common flags(e.g. CLK_SET_RATE_PARENT)
 * @data:		points to clk specific data(e.g. struct clk_regmap_gate_data *)
 *
 */
struct clk_common_data {
	const char *	name;
	union {
		const char *	parent_name;
		const char * const *parent_names;
	};
	u8		num_parents;
	ulong		flags;
	enum clk_type	type;
	void *		data;
};

/**
 * struct clk_regmap_gate_data - regmap backed gate specific data
 *
 * @name:	name of this clock
 * @parent_name:name of the parent clock
 * @offset:	offset of the register controlling gate
 * @bit_idx:	single bit controlling gate
 * @flags:	hardware-specific flags
 *
 * Flags:
 * Same as clk_gate except CLK_GATE_HIWORD_MASK which is ignored
 */
struct clk_regmap_gate_data {
	unsigned int	offset;
	u8		bit_idx;
	u8		flags;
};

/**
 * struct clk_regmap_div_data - regmap backed adjustable divider specific data
 *
 * @name:	name of this clock
 * @parent_name:name of the parent clock
 * @offset:	offset of the register controlling the divider
 * @shift:	shift to the divider bit field
 * @width:	width of the divider bit field
 * @table:	array of value/divider pairs, last entry should have div = 0
 *
 * Flags:
 * Same as clk_divider except CLK_DIVIDER_HIWORD_MASK which is ignored
 */
struct clk_regmap_div_data {
	unsigned int	offset;
	u8		shift;
	u8		width;
	u8		flags;
	const struct clk_div_table	*table;
};

/**
 * struct clk_regmap_mux_data - regmap backed multiplexer clock specific data
 *
 * @table:	array of parent indexed register values
 * @offset:	offset of theregister controlling multiplexer
 * @shift:	shift to multiplexer bit field
 * @mask:	mask of mutliplexer bit field
 * @flags:	hardware-specific flags
 *
 * Flags:
 * Same as clk_mux except CLK_MUX_HIWORD_MASK which is ignored
 */
struct clk_regmap_mux_data {
	u32 *			table;
	unsigned int		offset;
	u32			mask;
	u8			shift;
	u8			flags;
};

/**
 * struct clk_regmap_fixed_factor_data - regmap backed fixed factor clock specific data
 *
 * @mult:	multiplexer
 * @div:	divisor
 * @flags:	hardware-specific flags
 *
 * Flags:
 * Same as clk_fixed_factor
 */
struct clk_regmap_fixed_factor_data {
	unsigned int	mult;
	unsigned int	div;
	u8		flags;
};

#define MESON_PCLK(_name, _reg, _bit, _pname)			\
struct clk_common_data _name = {				\
	.name = #_name,						\
	.parent_name = #_pname,					\
	.flags = (CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED),	\
	.type = CLK_GATE,					\
	.data = &(struct clk_regmap_gate_data){			\
		.offset = (_reg),				\
		.bit_idx = (_bit),				\
	}							\
}

struct clk *meson_clk_register(struct udevice *dev, void __iomem *base, struct clk_common_data *clk);

#endif /* __CLK_REGMAP_H */
