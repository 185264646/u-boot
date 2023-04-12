/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Adapted from linux kernel, some functionalities are removed to keep it simple.
 */

#ifndef	__HISI_CLK_H
#define	__HISI_CLK_H

#include <common.h>
#include <clk.h>

struct device;

struct hisi_clock_data {
	void __iomem		*base;
};

struct hisi_fixed_rate_clock {
	unsigned int		id;
	char			*name;
	unsigned long		fixed_rate;
};

struct hisi_mux_clock {
	unsigned int		id;
	const char		*name;
	const char		*const *parent_names;
	u8			num_parents;
	unsigned long		flags;
	unsigned long		offset;
	u8			shift;
	u8			width;
	u8			mux_flags;
	u32			*table;
};

struct hisi_gate_clock {
	unsigned int		id;
	const char		*name;
	const char		*parent_name;
	unsigned long		flags;
	unsigned long		offset;
	u8			bit_idx;
	u8			gate_flags;
};

int hisi_clk_init(struct udevice *dev, struct hisi_clock_data *);
int hisi_clk_register_fixed_rate(struct device *, const struct hisi_fixed_rate_clock *, int nums);
int hisi_clk_register_mux(struct device *, const struct hisi_mux_clock *, int nums, const struct hisi_clock_data *);
int hisi_clk_register_gate(struct device *, const struct hisi_gate_clock *, int nums, const struct hisi_clock_data *);

#endif	/* __HISI_CLK_H */
