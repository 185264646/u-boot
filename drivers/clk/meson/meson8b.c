// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 Endless Mobile, Inc.
 * Author: Carlo Caione <carlo@endlessm.com>
 *
 * Copyright (c) 2016 BayLibre, Inc.
 * Michael Turquette <mturquette@baylibre.com>
 */

#include <dm/device.h>
#include <dm/read.h>
#include <linux/clk-provider.h>

#include "meson8b.h"
#include "clk-regmap.h"

#include <dt-bindings/clock/meson8b-clkc.h>
#include <dt-bindings/reset/amlogic,meson8b-clkc-reset.h>

// TODO
#if 0
struct meson8b_clk_reset {
	struct reset_controller_dev reset;
	struct regmap *regmap;
};

static const struct pll_params_table sys_pll_params_table[] = {
	PLL_PARAMS(50, 1),
	PLL_PARAMS(51, 1),
	PLL_PARAMS(52, 1),
	PLL_PARAMS(53, 1),
	PLL_PARAMS(54, 1),
	PLL_PARAMS(55, 1),
	PLL_PARAMS(56, 1),
	PLL_PARAMS(57, 1),
	PLL_PARAMS(58, 1),
	PLL_PARAMS(59, 1),
	PLL_PARAMS(60, 1),
	PLL_PARAMS(61, 1),
	PLL_PARAMS(62, 1),
	PLL_PARAMS(63, 1),
	PLL_PARAMS(64, 1),
	PLL_PARAMS(65, 1),
	PLL_PARAMS(66, 1),
	PLL_PARAMS(67, 1),
	PLL_PARAMS(68, 1),
	PLL_PARAMS(84, 1),
	{ /* sentinel */ },
};
#endif

static struct clk_common_data meson8b_fixed_pll_dco = {
	.name = "fixed_pll_dco",
	.parent_name = "xtal",
#if 0
	.type = CLK_PLL,
	.data = &(struct meson_clk_pll_data){
		.en = {
			.reg_off = HHI_MPLL_CNTL,
			.shift   = 30,
			.width   = 1,
		},
		.m = {
			.reg_off = HHI_MPLL_CNTL,
			.shift   = 0,
			.width   = 9,
		},
		.n = {
			.reg_off = HHI_MPLL_CNTL,
			.shift   = 9,
			.width   = 5,
		},
		.frac = {
			.reg_off = HHI_MPLL_CNTL2,
			.shift   = 0,
			.width   = 12,
		},
		.l = {
			.reg_off = HHI_MPLL_CNTL,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = HHI_MPLL_CNTL,
			.shift   = 29,
			.width   = 1,
		},
	},
#else
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		/* TODO: Implement a proper PLL driver */
		.div = 4,
		.mult = 425,
	}
#endif
};

static struct clk_common_data meson8b_fixed_pll = {
	.name = "fixed_pll",
	.parent_name = "fixed_pll_dco",
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data) {
		.offset = HHI_MPLL_CNTL,
		.shift = 16,
		.width = 2,
		.flags = CLK_DIVIDER_POWER_OF_TWO
	}
};

static struct clk_common_data hdmi_pll_dco_in = {
	.name = "hdmi_pll_dco_in",
	.parent_name = "xtal",
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 2,
		.div = 1
	}
};

/*
 * Taken from the vendor driver for the 2970/2975MHz (both only differ in the
 * FRAC part in HHI_VID_PLL_CNTL2) where these values are identical for Meson8,
 * Meson8b and Meson8m2. This doubles the input (or output - it's not clear
 * which one but the result is the same) clock. The vendor driver additionally
 * has the following comment about: "optimise HPLL VCO 2.97GHz performance".
 */
#if 0
static const struct reg_sequence meson8b_hdmi_pll_init_regs[] = {
	{ .reg = HHI_VID_PLL_CNTL2,	.def = 0x69c84000 },
	{ .reg = HHI_VID_PLL_CNTL3,	.def = 0x8a46c023 },
	{ .reg = HHI_VID_PLL_CNTL4,	.def = 0x4123b100 },
	{ .reg = HHI_VID_PLL_CNTL5,	.def = 0x00012385 },
	{ .reg = HHI_VID2_PLL_CNTL2,	.def = 0x0430a800 },
};

static const struct pll_params_table hdmi_pll_params_table[] = {
	PLL_PARAMS(40, 1),
	PLL_PARAMS(42, 1),
	PLL_PARAMS(44, 1),
	PLL_PARAMS(45, 1),
	PLL_PARAMS(49, 1),
	PLL_PARAMS(52, 1),
	PLL_PARAMS(54, 1),
	PLL_PARAMS(56, 1),
	PLL_PARAMS(59, 1),
	PLL_PARAMS(60, 1),
	PLL_PARAMS(61, 1),
	PLL_PARAMS(62, 1),
	PLL_PARAMS(64, 1),
	PLL_PARAMS(66, 1),
	PLL_PARAMS(68, 1),
	PLL_PARAMS(71, 1),
	PLL_PARAMS(82, 1),
	{ /* sentinel */ }
};

static struct clk_regmap meson8b_hdmi_pll_dco = {
	.data = &(struct meson_clk_pll_data){
		.en = {
			.reg_off = HHI_VID_PLL_CNTL,
			.shift   = 30,
			.width   = 1,
		},
		.m = {
			.reg_off = HHI_VID_PLL_CNTL,
			.shift   = 0,
			.width   = 9,
		},
		.n = {
			.reg_off = HHI_VID_PLL_CNTL,
			.shift   = 10,
			.width   = 5,
		},
		.frac = {
			.reg_off = HHI_VID_PLL_CNTL2,
			.shift   = 0,
			.width   = 12,
		},
		.l = {
			.reg_off = HHI_VID_PLL_CNTL,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = HHI_VID_PLL_CNTL,
			.shift   = 29,
			.width   = 1,
		},
		.table = hdmi_pll_params_table,
		.init_regs = meson8b_hdmi_pll_init_regs,
		.init_count = ARRAY_SIZE(meson8b_hdmi_pll_init_regs),
	},
	/* sometimes also called "HPLL" or "HPLL PLL" */
	.name = "hdmi_pll_dco",
	.parent_hws = (const struct clk_hw *[]) {
		&hdmi_pll_dco_in.hw
	},
};
#endif

static struct clk_common_data meson8b_hdmi_pll_lvds_out = {
	.name = "hdmi_pll_lvds_out",
	.parent_name =  "hdmi_pll_dco",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset = HHI_VID_PLL_CNTL,
		.shift = 16,
		.width = 2,
		.flags = CLK_DIVIDER_POWER_OF_TWO,
	}
};

static struct clk_common_data meson8b_hdmi_pll_hdmi_out = {
	.name = "hdmi_pll_hdmi_out",
	.parent_name =  "hdmi_pll_dco",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data) {
		.offset = HHI_VID_PLL_CNTL,
		.shift = 18,
		.width = 2,
		.flags = CLK_DIVIDER_POWER_OF_TWO
	}
};

#if 0
static struct clk_regmap meson8b_sys_pll_dco = {
	.data = &(struct meson_clk_pll_data){
		.en = {
			.reg_off = HHI_SYS_PLL_CNTL,
			.shift   = 30,
			.width   = 1,
		},
		.m = {
			.reg_off = HHI_SYS_PLL_CNTL,
			.shift   = 0,
			.width   = 9,
		},
		.n = {
			.reg_off = HHI_SYS_PLL_CNTL,
			.shift   = 9,
			.width   = 5,
		},
		.l = {
			.reg_off = HHI_SYS_PLL_CNTL,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = HHI_SYS_PLL_CNTL,
			.shift   = 29,
			.width   = 1,
		},
		.table = sys_pll_params_table,
	},
	.name = "sys_pll_dco",
	.parent_data = &(const struct clk_parent_data) {
		.fw_name = "xtal",
		.name = "xtal",
		.index = -1,
	},
};
#endif

static struct clk_common_data meson8b_sys_pll = {
	.name = "sys_pll",
	.parent_name = "sys_pll_dco",
	.type = CLK_DIVIDER,
	.flags = CLK_SET_RATE_PARENT,
	.data = &(struct clk_regmap_div_data){
		.offset = HHI_SYS_PLL_CNTL,
		.shift = 16,
		.width = 2,
		.flags = CLK_DIVIDER_POWER_OF_TWO
	}
};

static struct clk_common_data meson8b_fclk_div2_div = {
	.name = "fclk_div2_div",
	.parent_name = "fixed_pll",
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 2
	}
};

static struct clk_common_data meson8b_fclk_div2 = {
	.name = "fclk_div2",
	.parent_name = "fclk_div2_div",
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_MPLL_CNTL6,
		.bit_idx = 27,
	},
};

static struct clk_common_data meson8b_fclk_div3_div = {
	.name = "fclk_div3_div",
	.parent_name = "fixed_pll",
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 3,
	},
};

static struct clk_common_data meson8b_fclk_div3 = {
	.name = "fclk_div3",
	.parent_name = "fclk_div3_div",
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_MPLL_CNTL6,
		.bit_idx = 28,
	},
};

static struct clk_common_data meson8b_fclk_div4_div = {
	.name = "fclk_div4_div",
	.parent_name = "fixed_pll",
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 4,
	},
};

static struct clk_common_data meson8b_fclk_div4 = {
	.name = "fclk_div4",
	.parent_name = "fclk_div4_div",
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_MPLL_CNTL6,
		.bit_idx = 29,
	},
};

static struct clk_common_data meson8b_fclk_div5_div = {
	.name = "fclk_div5_div",
	.parent_name = "fixed_pll",
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 5,
	},
};

static struct clk_common_data meson8b_fclk_div5 = {
	.name = "fclk_div5",
	.parent_name = "fclk_div5_div",
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_MPLL_CNTL6,
		.bit_idx = 30,
	},
};

static struct clk_common_data meson8b_fclk_div7_div = {
	.name = "fclk_div7_div",
	.parent_name = "fixed_pll",
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 7,
	},
};

static struct clk_common_data meson8b_fclk_div7 = {
	.name = "fclk_div7",
	.parent_name = "fclk_div7_div",
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_MPLL_CNTL6,
		.bit_idx = 31,
	},
};

static struct clk_common_data meson8b_mpll_prediv = {
	.name = "mpll_prediv",
	.parent_name = "fixed_pll",
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset = HHI_MPLL_CNTL5,
		.shift = 12,
		.width = 1,
	},
};

#if 0
static struct clk_common_data meson8b_mpll0_div = {
	.data = &(struct meson_clk_mpll_data){
		.sdm = {
			.reg_off = HHI_MPLL_CNTL7,
			.shift   = 0,
			.width   = 14,
		},
		.sdm_en = {
			.reg_off = HHI_MPLL_CNTL7,
			.shift   = 15,
			.width   = 1,
		},
		.n2 = {
			.reg_off = HHI_MPLL_CNTL7,
			.shift   = 16,
			.width   = 9,
		},
		.ssen = {
			.reg_off = HHI_MPLL_CNTL,
			.shift   = 25,
			.width   = 1,
		},
	},
	.name = "mpll0_div",
	.parent_name = "mpll_prediv",
};
#endif

static struct clk_common_data meson8b_mpll0 = {
	.name = "mpll0",
	.parent_name = "mpll0_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_MPLL_CNTL7,
		.bit_idx = 14,
	},
};

#if 0
static struct clk_common_data meson8b_mpll1_div = {
	.data = &(struct meson_clk_mpll_data){
		.sdm = {
			.reg_off = HHI_MPLL_CNTL8,
			.shift   = 0,
			.width   = 14,
		},
		.sdm_en = {
			.reg_off = HHI_MPLL_CNTL8,
			.shift   = 15,
			.width   = 1,
		},
		.n2 = {
			.reg_off = HHI_MPLL_CNTL8,
			.shift   = 16,
			.width   = 9,
		},
	},
	.name = "mpll1_div",
	.parent_name = "mpll_prediv",
};
#endif

static struct clk_common_data meson8b_mpll1 = {
	.name = "mpll1",
	.parent_name = "mpll1_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_MPLL_CNTL8,
		.bit_idx = 14,
	},
};

#if 0
static struct clk_common_data meson8b_mpll2_div = {
	.data = &(struct meson_clk_mpll_data){
		.sdm = {
			.reg_off = HHI_MPLL_CNTL9,
			.shift   = 0,
			.width   = 14,
		},
		.sdm_en = {
			.reg_off = HHI_MPLL_CNTL9,
			.shift   = 15,
			.width   = 1,
		},
		.n2 = {
			.reg_off = HHI_MPLL_CNTL9,
			.shift   = 16,
			.width   = 9,
		},
	},
	.name = "mpll2_div",
	.parent_name = "mpll_prediv",
};
#endif

static struct clk_common_data meson8b_mpll2 = {
	.name = "mpll2",
	.parent_name = "mpll2_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_MPLL_CNTL9,
		.bit_idx = 14,
	},
};

static u32 mux_table_clk81[]	= { 6, 5, 7 };
static struct clk_common_data meson8b_mpeg_clk_sel = {
	.name = "mpeg_clk_sel",
	/*
	 * FIXME bits 14:12 selects from 8 possible parents:
	 * xtal, 1'b0 (wtf), fclk_div7, mpll_clkout1, mpll_clkout2,
	 * fclk_div4, fclk_div3, fclk_div5
	 */
	.parent_names = (const char * const[]) {
		"fclk_div3",
		"fclk_div4",
		"fclk_div5",
	},
	.num_parents = 3,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_MPEG_CLK_CNTL,
		.mask = 0x7,
		.shift = 12,
		.table = mux_table_clk81,
	},
};

static struct clk_common_data meson8b_mpeg_clk_div = {
	.name = "mpeg_clk_div",
	.parent_name = "mpeg_clk_sel",
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset = HHI_MPEG_CLK_CNTL,
		.shift = 0,
		.width = 7,
	},
};

static struct clk_common_data meson8b_clk81 = {
	.name = "clk81",
	.parent_name = "mpeg_clk_div",
	.flags = CLK_IS_CRITICAL,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_MPEG_CLK_CNTL,
		.bit_idx = 7,
	},
};

static struct clk_common_data meson8b_cpu_in_sel = {
	.name = "cpu_in_sel",
	.parent_names = (const char *const []) {
		"xtal",
		"sys_pll",
	},
	.num_parents = 2,
	.flags = (CLK_SET_RATE_PARENT |
		  CLK_SET_RATE_NO_REPARENT),
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_SYS_CPU_CLK_CNTL0,
		.mask = 0x1,
		.shift = 0,
	},
};

static struct clk_common_data meson8b_cpu_in_div2 = {
	.name = "cpu_in_div2",
	.parent_name = "cpu_in_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 2,
	},
};

static struct clk_common_data meson8b_cpu_in_div3 = {
	.name = "cpu_in_div3",
	.parent_name = "cpu_in_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 3,
	},
};

static const struct clk_div_table cpu_scale_table[] = {
	{ .val = 1, .div = 4 },
	{ .val = 2, .div = 6 },
	{ .val = 3, .div = 8 },
	{ .val = 4, .div = 10 },
	{ .val = 5, .div = 12 },
	{ .val = 6, .div = 14 },
	{ .val = 7, .div = 16 },
	{ .val = 8, .div = 18 },
	{ /* sentinel */ },
};

static struct clk_common_data meson8b_cpu_scale_div = {
	.name = "cpu_scale_div",
	.parent_name = "cpu_in_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset =  HHI_SYS_CPU_CLK_CNTL1,
		.shift = 20,
		.width = 10,
		.table = cpu_scale_table,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
};

static u32 mux_table_cpu_scale_out_sel[] = { 0, 1, 3 };
static struct clk_common_data meson8b_cpu_scale_out_sel = {
	.name = "cpu_scale_out_sel",
	/*
	 * NOTE: We are skipping the parent with value 0x2 (which is
	 * meson8b_cpu_in_div3) because it results in a duty cycle of
	 * 33% which makes the system unstable and can result in a
	 * lockup of the whole system.
	 */
	.parent_names = (const char *const []) {
		"cpu_in_sel",
		"cpu_in_div2",
		"cpu_scale_div",
	},
	.num_parents = 3,
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.table = mux_table_cpu_scale_out_sel,
		.offset = HHI_SYS_CPU_CLK_CNTL0,
		.mask = 0x3,
		.shift = 2,
	},
};

static u32 mux_table_cpu_clk[] = { 0, 1 };
static struct clk_common_data meson8b_cpu_clk = {
	.name = "cpu_clk",
	.parent_names = (const char *const []) {
		"xtal",
		"cpu_scale_out_sel"
	},
	.num_parents = 2,
	.flags = (CLK_SET_RATE_PARENT |
		  CLK_SET_RATE_NO_REPARENT |
		  CLK_IS_CRITICAL),
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.table = mux_table_cpu_clk,
		.offset = HHI_SYS_CPU_CLK_CNTL0,
		.mask = 0x1,
		.shift = 7,
	},
};

static struct clk_common_data meson8b_nand_clk_sel = {
	.name = "nand_clk_sel",
	/* FIXME all other parents are unknown: */
	.parent_names = (const char *const []) {
		"meson8b_fclk_div4",
		"meson8b_fclk_div3",
		"meson8b_fclk_div5",
		"meson8b_fclk_div7",
		"xtal",
	},
	.num_parents = 5,
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.table = (u32[]){ 0, 1, 2, 3, 4 },
		.offset = HHI_NAND_CLK_CNTL,
		.mask = 0x7,
		.shift = 9,
		.flags = CLK_MUX_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_nand_clk_div = {
	.name = "nand_clk_div",
	.parent_name = "nand_clk_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset =  HHI_NAND_CLK_CNTL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_nand_clk_gate = {
	.name = "nand_clk_gate",
	.parent_name = "nand_clk_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_NAND_CLK_CNTL,
		.bit_idx = 8,
	},
};

static struct clk_common_data meson8b_cpu_clk_div2 = {
	.name = "cpu_clk_div2",
	.parent_name = "cpu_clk",
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 2,
	},
};

static struct clk_common_data meson8b_cpu_clk_div3 = {
	.name = "cpu_clk_div3",
	.parent_name = "cpu_clk",
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 3,
	},
};

static struct clk_common_data meson8b_cpu_clk_div4 = {
	.name = "cpu_clk_div4",
	.parent_name = "cpu_clk",
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 4,
	},
};

static struct clk_common_data meson8b_cpu_clk_div5 = {
	.name = "cpu_clk_div5",
	.parent_name = "cpu_clk",
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 5,
	},
};

static struct clk_common_data meson8b_cpu_clk_div6 = {
	.name = "cpu_clk_div6",
	.parent_name = "cpu_clk",
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 6,
	},
};

static struct clk_common_data meson8b_cpu_clk_div7 = {
	.name = "cpu_clk_div7",
	.parent_name = "cpu_clk",
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 7,
	},
};

static struct clk_common_data meson8b_cpu_clk_div8 = {
	.name = "cpu_clk_div8",
	.parent_name = "cpu_clk",
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 8,
	},
};

static u32 mux_table_apb[] = { 1, 2, 3, 4, 5, 6, 7 };
static struct clk_common_data meson8b_apb_clk_sel = {
	.name = "apb_clk_sel",
	.parent_names = (const char *const []) {
		"cpu_clk_div2",
		"cpu_clk_div3",
		"cpu_clk_div4",
		"cpu_clk_div5",
		"cpu_clk_div6",
		"cpu_clk_div7",
		"cpu_clk_div8",
	},
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.table = mux_table_apb,
		.offset = HHI_SYS_CPU_CLK_CNTL1,
		.mask = 0x7,
		.shift = 3,
	},
};

static struct clk_common_data meson8b_apb_clk_gate = {
	.name = "apb_clk_dis",
	.parent_name = "apb_clk_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_SYS_CPU_CLK_CNTL1,
		.bit_idx = 16,
		.flags = CLK_GATE_SET_TO_DISABLE,
	},
};

static struct clk_common_data meson8b_periph_clk_sel = {
	.name = "periph_clk_sel",
	.parent_names = (const char *const []) {
		"cpu_clk_div2",
		"cpu_clk_div3",
		"cpu_clk_div4",
		"cpu_clk_div5",
		"cpu_clk_div6",
		"cpu_clk_div7",
		"cpu_clk_div8",
	},
	.num_parents = 7,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_SYS_CPU_CLK_CNTL1,
		.mask = 0x7,
		.shift = 6,
	},
};

static struct clk_common_data meson8b_periph_clk_gate = {
	.name = "periph_clk_dis",
	.parent_name = "periph_clk_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_SYS_CPU_CLK_CNTL1,
		.bit_idx = 17,
		.flags = CLK_GATE_SET_TO_DISABLE,
	},
};

static u32 mux_table_axi[] = { 1, 2, 3, 4, 5, 6, 7 };
static struct clk_common_data meson8b_axi_clk_sel = {
	.name = "axi_clk_sel",
	.parent_names = (const char *const []) {
		"cpu_clk_div2",
		"cpu_clk_div3",
		"cpu_clk_div4",
		"cpu_clk_div5",
		"cpu_clk_div6",
		"cpu_clk_div7",
		"cpu_clk_div8",
	},
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.table = mux_table_axi,
		.offset = HHI_SYS_CPU_CLK_CNTL1,
		.mask = 0x7,
		.shift = 9,
	},
};

static struct clk_common_data meson8b_axi_clk_gate = {
	.name = "axi_clk_dis",
	.parent_name = "axi_clk_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_SYS_CPU_CLK_CNTL1,
		.bit_idx = 18,
		.flags = CLK_GATE_SET_TO_DISABLE,
	},
};

static struct clk_common_data meson8b_l2_dram_clk_sel = {
	.name = "l2_dram_clk_sel",
	.parent_names = (const char *const []) {
		"cpu_clk_div2",
		"cpu_clk_div3",
		"cpu_clk_div4",
		"cpu_clk_div5",
		"cpu_clk_div6",
		"cpu_clk_div7",
		"cpu_clk_div8",
	},
	.num_parents = 7,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_SYS_CPU_CLK_CNTL1,
		.mask = 0x7,
		.shift = 12,
	},
};

static struct clk_common_data meson8b_l2_dram_clk_gate = {
	.name = "l2_dram_clk_dis",
	.parent_name = "l2_dram_clk_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_SYS_CPU_CLK_CNTL1,
		.bit_idx = 19,
		.flags = CLK_GATE_SET_TO_DISABLE,
	},
};

/* also called LVDS_CLK_EN */
static struct clk_common_data meson8b_vid_pll_lvds_en = {
	.name = "vid_pll_lvds_en",
	.parent_name = "hdmi_pll_lvds_out",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VID_DIVIDER_CNTL,
		.bit_idx = 11,
	},
};

static struct clk_common_data meson8b_vid_pll_in_sel = {
	.name = "vid_pll_in_sel",
	/*
	 * TODO: depending on the SoC there is also a second parent:
	 * Meson8: unknown
	 * Meson8b: hdmi_pll_dco
	 * Meson8m2: vid2_pll
	 */
	.parent_names = (const char *const []){ "vid_pll_lvds_en" },
	.num_parents = 1,
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.table = (u32[]){ 0 },
		.offset = HHI_VID_DIVIDER_CNTL,
		.mask = 0x1,
		.shift = 15,
	},
};

static struct clk_common_data meson8b_vid_pll_in_en = {
	.name = "vid_pll_in_en",
	.parent_name = "vid_pll_in_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VID_DIVIDER_CNTL,
		.bit_idx = 16,
	},
};

static struct clk_common_data meson8b_vid_pll_pre_div = {
	.name = "vid_pll_pre_div",
	.parent_name = "vid_pll_in_en",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset =  HHI_VID_DIVIDER_CNTL,
		.shift = 4,
		.width = 3,
	},
};

static struct clk_common_data meson8b_vid_pll_post_div = {
	.name = "vid_pll_post_div",
	.parent_name = "vid_pll_pre_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset =  HHI_VID_DIVIDER_CNTL,
		.shift = 12,
		.width = 3,
	},
};

static struct clk_common_data meson8b_vid_pll = {
	.name = "vid_pll",
	/* TODO: parent 0x2 is vid_pll_pre_div_mult7_div2 */
	.parent_names = (const char *const []) {
		"vid_pll_pre_div",
		"vid_pll_post_div",
	},
	.num_parents = 2,
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VID_DIVIDER_CNTL,
		.mask = 0x3,
		.shift = 8,
	},
};

static struct clk_common_data meson8b_vid_pll_final_div = {
	.name = "vid_pll_final_div",
	.parent_name = "vid_pll",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset =  HHI_VID_CLK_DIV,
		.shift = 0,
		.width = 8,
	},
};

static const char * meson8b_vclk_mux_parent_names[] = {
	"vid_pll_final_div",
	"fclk_div4",
	"fclk_div3",
	"fclk_div5",
	"vid_pll_final_div",
	"fclk_div7",
	"mpll1",
};

static struct clk_common_data meson8b_vclk_in_sel = {
	.name = "vclk_in_sel",
	.parent_names = meson8b_vclk_mux_parent_names,
	.num_parents = ARRAY_SIZE(meson8b_vclk_mux_parent_names),
	.flags = CLK_SET_RATE_PARENT | CLK_SET_RATE_NO_REPARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VID_CLK_CNTL,
		.mask = 0x7,
		.shift = 16,
	},
};

static struct clk_common_data meson8b_vclk_in_en = {
	.name = "vclk_in_en",
	.parent_name = "vclk_in_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VID_CLK_DIV,
		.bit_idx = 16,
	},
};

static struct clk_common_data meson8b_vclk_en = {
	.name = "vclk_en",
	.parent_name = "vclk_in_en",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VID_CLK_CNTL,
		.bit_idx = 19,
	},
};

static struct clk_common_data meson8b_vclk_div1_gate = {
	.name = "vclk_div1_en",
	.parent_name = "vclk_en",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VID_CLK_CNTL,
		.bit_idx = 0,
	},
};

static struct clk_common_data meson8b_vclk_div2_div = {
	.name = "vclk_div2",
	.parent_name = "vclk_en",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 2,
	},
};

static struct clk_common_data meson8b_vclk_div2_div_gate = {
	.name = "vclk_div2_en",
	.parent_name = "vclk_div2_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VID_CLK_CNTL,
		.bit_idx = 1,
	},
};

static struct clk_common_data meson8b_vclk_div4_div = {
	.name = "vclk_div4",
	.parent_name = "vclk_en",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 4,
	},
};

static struct clk_common_data meson8b_vclk_div4_div_gate = {
	.name = "vclk_div4_en",
	.parent_name = "vclk_div4_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VID_CLK_CNTL,
		.bit_idx = 2,
	},
};

static struct clk_common_data meson8b_vclk_div6_div = {
	.name = "vclk_div6",
	.parent_name = "vclk_en",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 6,
	},
};

static struct clk_common_data meson8b_vclk_div6_div_gate = {
	.name = "vclk_div6_en",
	.parent_name = "vclk_div6_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VID_CLK_CNTL,
		.bit_idx = 3,
	},
};

static struct clk_common_data meson8b_vclk_div12_div = {
	.name = "vclk_div12",
	.parent_name = "vclk_en",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 12,
	},
};

static struct clk_common_data meson8b_vclk_div12_div_gate = {
	.name = "vclk_div12_en",
	.parent_name = "vclk_div12_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VID_CLK_CNTL,
		.bit_idx = 4,
	},
};

static struct clk_common_data meson8b_vclk2_in_sel = {
	.name = "vclk2_in_sel",
	.parent_names = meson8b_vclk_mux_parent_names,
	.num_parents = ARRAY_SIZE(meson8b_vclk_mux_parent_names),
	.flags = CLK_SET_RATE_PARENT | CLK_SET_RATE_NO_REPARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VIID_CLK_CNTL,
		.mask = 0x7,
		.shift = 16,
	},
};

static struct clk_common_data meson8b_vclk2_clk_in_en = {
	.name = "vclk2_in_en",
	.parent_name = "vclk2_in_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VIID_CLK_DIV,
		.bit_idx = 16,
	},
};

static struct clk_common_data meson8b_vclk2_clk_en = {
	.name = "vclk2_en",
	.parent_name = "vclk2_clk_in_en",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VIID_CLK_DIV,
		.bit_idx = 19,
	},
};

static struct clk_common_data meson8b_vclk2_div1_gate = {
	.name = "vclk2_div1_en",
	.parent_name = "vclk2_clk_en",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VIID_CLK_DIV,
		.bit_idx = 0,
	},
};

static struct clk_common_data meson8b_vclk2_div2_div = {
	.name = "vclk2_div2",
	.parent_name = "vclk2_clk_en",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 2,
	},
};

static struct clk_common_data meson8b_vclk2_div2_div_gate = {
	.name = "vclk2_div2_en",
	.parent_name = "vclk2_div2_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VIID_CLK_DIV,
		.bit_idx = 1,
	},
};

static struct clk_common_data meson8b_vclk2_div4_div = {
	.name = "vclk2_div4",
	.parent_name = "vclk2_clk_en",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 4,
	},
};

static struct clk_common_data meson8b_vclk2_div4_div_gate = {
	.name = "vclk2_div4_en",
	.parent_name = "vclk2_div4_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VIID_CLK_DIV,
		.bit_idx = 2,
	},
};

static struct clk_common_data meson8b_vclk2_div6_div = {
	.name = "vclk2_div6",
	.parent_name = "vclk2_clk_en",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 6,
	},
};

static struct clk_common_data meson8b_vclk2_div6_div_gate = {
	.name = "vclk2_div6_en",
	.parent_name = "vclk2_div6_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VIID_CLK_DIV,
		.bit_idx = 3,
	},
};

static struct clk_common_data meson8b_vclk2_div12_div = {
	.name = "vclk2_div12",
	.parent_name = "vclk2_clk_en",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_FIXED_FACTOR,
	.data = &(struct clk_regmap_fixed_factor_data) {
		.mult = 1,
		.div = 12,
	},
};

static struct clk_common_data meson8b_vclk2_div12_div_gate = {
	.name = "vclk2_div12_en",
	.parent_name = "vclk2_div12_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VIID_CLK_DIV,
		.bit_idx = 4,
	},
};

static const char *meson8b_vclk_enc_mux_parent_names[] = {
	"vclk_div1_gate",
	"vclk_div2_div_gate",
	"vclk_div4_div_gate",
	"vclk_div6_div_gate",
	"vclk_div12_div_gate",
};

static struct clk_common_data meson8b_cts_enct_sel = {
	.name = "cts_enct_sel",
	.parent_names = meson8b_vclk_enc_mux_parent_names,
	.num_parents = ARRAY_SIZE(meson8b_vclk_enc_mux_parent_names),
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VID_CLK_DIV,
		.mask = 0xf,
		.shift = 20,
	},
};

static struct clk_common_data meson8b_cts_enct = {
	.name = "cts_enct",
	.parent_name = "cts_enct_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VID_CLK_CNTL2,
		.bit_idx = 1,
	},
};

static struct clk_common_data meson8b_cts_encp_sel = {
	.name = "cts_encp_sel",
	.parent_names = meson8b_vclk_enc_mux_parent_names,
	.num_parents = ARRAY_SIZE(meson8b_vclk_enc_mux_parent_names),
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VID_CLK_DIV,
		.mask = 0xf,
		.shift = 24,
	},
};

static struct clk_common_data meson8b_cts_encp = {
	.name = "cts_encp",
	.parent_name = "cts_encp_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VID_CLK_CNTL2,
		.bit_idx = 2,
	},
};

static struct clk_common_data meson8b_cts_enci_sel = {
	.name = "cts_enci_sel",
	.parent_names = meson8b_vclk_enc_mux_parent_names,
	.num_parents = ARRAY_SIZE(meson8b_vclk_enc_mux_parent_names),
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VID_CLK_DIV,
		.mask = 0xf,
		.shift = 28,
	},
};

static struct clk_common_data meson8b_cts_enci = {
	.name = "cts_enci",
	.parent_name = "cts_enci_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VID_CLK_CNTL2,
		.bit_idx = 0,
	},
};

static struct clk_common_data meson8b_hdmi_tx_pixel_sel = {
	.name = "hdmi_tx_pixel_sel",
	.parent_names = meson8b_vclk_enc_mux_parent_names,
	.num_parents = ARRAY_SIZE(meson8b_vclk_enc_mux_parent_names),
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_HDMI_CLK_CNTL,
		.mask = 0xf,
		.shift = 16,
	},
};

static struct clk_common_data meson8b_hdmi_tx_pixel = {
	.name = "hdmi_tx_pixel",
	.parent_name = "hdmi_tx_pixel_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VID_CLK_CNTL2,
		.bit_idx = 5,
	},
};

static const char *meson8b_vclk2_enc_mux_parent_names[] = {
	"vclk2_div1_gate",
	"vclk2_div2_div_gate",
	"vclk2_div4_div_gate",
	"vclk2_div6_div_gate",
	"vclk2_div12_div_gate",
};

static struct clk_common_data meson8b_cts_encl_sel = {
	.name = "cts_encl_sel",
	.parent_names = meson8b_vclk2_enc_mux_parent_names,
	.num_parents = ARRAY_SIZE(meson8b_vclk2_enc_mux_parent_names),
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VIID_CLK_DIV,
		.mask = 0xf,
		.shift = 12,
	},
};

static struct clk_common_data meson8b_cts_encl = {
	.name = "cts_encl",
	.parent_name = "cts_encl_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VID_CLK_CNTL2,
		.bit_idx = 3,
	},
};

static struct clk_common_data meson8b_cts_vdac0_sel = {
	.name = "cts_vdac0_sel",
	.parent_names = meson8b_vclk2_enc_mux_parent_names,
	.num_parents = ARRAY_SIZE(meson8b_vclk2_enc_mux_parent_names),
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VIID_CLK_DIV,
		.mask = 0xf,
		.shift = 28,
	},
};

static struct clk_common_data meson8b_cts_vdac0 = {
	.name = "cts_vdac0",
	.parent_name = "cts_vdac0_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VID_CLK_CNTL2,
		.bit_idx = 4,
	},
};

static struct clk_common_data meson8b_hdmi_sys_sel = {
	.name = "hdmi_sys_sel",
	/* FIXME: all other parents are unknown */
	.parent_names = (const char *const []){ "xtal" },
	.num_parents = 1,
	.flags = CLK_SET_RATE_NO_REPARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_HDMI_CLK_CNTL,
		.mask = 0x3,
		.shift = 9,
		.flags = CLK_MUX_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_hdmi_sys_div = {
	.name = "hdmi_sys_div",
	.parent_name = "hdmi_sys_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset = HHI_HDMI_CLK_CNTL,
		.shift = 0,
		.width = 7,
	},
};

static struct clk_common_data meson8b_hdmi_sys = {
	.name = "hdmi_sys",
	.parent_name = "hdmi_sys_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_HDMI_CLK_CNTL,
		.bit_idx = 8,
	},
};

/*
 * The MALI IP is clocked by two identical clocks (mali_0 and mali_1)
 * muxed by a glitch-free switch on Meson8b and Meson8m2. The CCF can
 * actually manage this glitch-free mux because it does top-to-bottom
 * updates the each clock tree and switches to the "inactive" one when
 * CLK_SET_RATE_GATE is set.
 * Meson8 only has mali_0 and no glitch-free mux.
 */
static const char *meson8b_mali_0_1_parent_names[] = {
	"xtal",
	"mpll2",
	"mpll1",
	"fclk_div7",
	"fclk_div4",
	"fclk_div3",
	"fclk_div5",
};

static u32 meson8b_mali_0_1_mux_table[] = { 0, 2, 3, 4, 5, 6, 7 };

static struct clk_common_data meson8b_mali_0_sel = {
	.name = "mali_0_sel",
	.parent_names = meson8b_mali_0_1_parent_names,
	/*
	 * Don't propagate rate changes up because the only changeable
	 * parents are mpll1 and mpll2 but we need those for audio and
	 * RGMII (Ethernet). We don't want to change the audio or
	 * Ethernet clocks when setting the GPU frequency.
	 */
	.num_parents = ARRAY_SIZE(meson8b_mali_0_1_parent_names),
	.flags = 0,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_MALI_CLK_CNTL,
		.mask = 0x7,
		.shift = 9,
		.table = meson8b_mali_0_1_mux_table,
	},
};

static struct clk_common_data meson8b_mali_0_div = {
	.name = "mali_0_div",
	.parent_name = "mali_0_sel",
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset = HHI_MALI_CLK_CNTL,
		.shift = 0,
		.width = 7,
	},
	.flags = CLK_SET_RATE_PARENT,
};

static struct clk_common_data meson8b_mali_0 = {
	.name = "mali_0",
	.parent_name = "mali_0_div",
	.flags = CLK_SET_RATE_GATE | CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_MALI_CLK_CNTL,
		.bit_idx = 8,
	},
};

static struct clk_common_data meson8b_mali_1_sel = {
	.name = "mali_1_sel",
	.parent_names = meson8b_mali_0_1_parent_names,
	/*
	 * Don't propagate rate changes up because the only changeable
	 * parents are mpll1 and mpll2 but we need those for audio and
	 * RGMII (Ethernet). We don't want to change the audio or
	 * Ethernet clocks when setting the GPU frequency.
	 */
	.num_parents = ARRAY_SIZE(meson8b_mali_0_1_parent_names),
	.flags = 0,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_MALI_CLK_CNTL,
		.mask = 0x7,
		.shift = 25,
		.table = meson8b_mali_0_1_mux_table,
	},
};

static struct clk_common_data meson8b_mali_1_div = {
	.name = "mali_1_div",
	.parent_name = "mali_1_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset = HHI_MALI_CLK_CNTL,
		.shift = 16,
		.width = 7,
	},
};

static struct clk_common_data meson8b_mali_1 = {
	.name = "mali_1",
	.parent_name = "mali_1_div",
	.flags = CLK_SET_RATE_GATE | CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_MALI_CLK_CNTL,
		.bit_idx = 24,
	},
};

static struct clk_common_data meson8b_mali = {
	.name = "mali",
	.parent_names = (const char *const []) {
		"mali_0",
		"mali_1",
	},
	.num_parents = 2,
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_MALI_CLK_CNTL,
		.mask = 1,
		.shift = 31,
	},
};

#if 0
static const struct reg_sequence meson8m2_gp_pll_init_regs[] = {
	{ .reg = HHI_GP_PLL_CNTL2,	.def = 0x59c88000 },
	{ .reg = HHI_GP_PLL_CNTL3,	.def = 0xca463823 },
	{ .reg = HHI_GP_PLL_CNTL4,	.def = 0x0286a027 },
	{ .reg = HHI_GP_PLL_CNTL5,	.def = 0x00003000 },
};

static const struct pll_params_table meson8m2_gp_pll_params_table[] = {
	PLL_PARAMS(182, 3),
	{ /* sentinel */ },
};

static struct clk_regmap meson8m2_gp_pll_dco = {
	.data = &(struct meson_clk_pll_data){
		.en = {
			.reg_off = HHI_GP_PLL_CNTL,
			.shift   = 30,
			.width   = 1,
		},
		.m = {
			.reg_off = HHI_GP_PLL_CNTL,
			.shift   = 0,
			.width   = 9,
		},
		.n = {
			.reg_off = HHI_GP_PLL_CNTL,
			.shift   = 9,
			.width   = 5,
		},
		.l = {
			.reg_off = HHI_GP_PLL_CNTL,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = HHI_GP_PLL_CNTL,
			.shift   = 29,
			.width   = 1,
		},
		.table = meson8m2_gp_pll_params_table,
		.init_regs = meson8m2_gp_pll_init_regs,
		.init_count = ARRAY_SIZE(meson8m2_gp_pll_init_regs),
	},
	.name = "gp_pll_dco",
	.parent_data = &(const struct clk_parent_data) {
		.fw_name = "xtal",
		.name = "xtal",
		.index = -1,
	},
};
#endif

static struct clk_common_data meson8m2_gp_pll = {
	.name = "gp_pll",
	.parent_name = "gp_pll_dco",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset = HHI_GP_PLL_CNTL,
		.shift = 16,
		.width = 2,
		.flags = CLK_DIVIDER_POWER_OF_TWO,
	},
};

static const char *meson8b_vpu_0_1_parent_names[] = {
	"fclk_div4",
	"fclk_div3",
	"fclk_div5",
	"fclk_div7",
};

static const char *meson8m2_vpu_0_1_parent_names[] = {
	"fclk_div4",
	"fclk_div3",
	"fclk_div5",
	"gp_pll",
};

static struct clk_common_data meson8b_vpu_0_sel = {
	.name = "vpu_0_sel",
	.parent_names = meson8b_vpu_0_1_parent_names,
	.num_parents = ARRAY_SIZE(meson8b_vpu_0_1_parent_names),
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VPU_CLK_CNTL,
		.mask = 0x3,
		.shift = 9,
	},
};

static struct clk_common_data meson8m2_vpu_0_sel = {
	.name = "vpu_0_sel",
	.parent_names = meson8m2_vpu_0_1_parent_names,
	.num_parents = ARRAY_SIZE(meson8m2_vpu_0_1_parent_names),
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VPU_CLK_CNTL,
		.mask = 0x3,
		.shift = 9,
	},
};

static struct clk_common_data meson8b_vpu_0_div = {
	.name = "vpu_0_div",
	.parent_name = "vpu_0_sel",
	/*
	 * Note:
	 * meson8b and meson8m2 have different vpu_0_sels (with
	 * different struct clk_hw). We fallback to the global
	 * naming string mechanism so vpu_0_div picks up the
	 * appropriate one.
	 */
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset = HHI_VPU_CLK_CNTL,
		.shift = 0,
		.width = 7,
	},
};

static struct clk_common_data meson8b_vpu_0 = {
	.name = "vpu_0",
	.parent_name = "vpu_0_div",
	.flags = CLK_SET_RATE_GATE | CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VPU_CLK_CNTL,
		.bit_idx = 8,
	},
};

static struct clk_common_data meson8b_vpu_1_sel = {
	.name = "vpu_1_sel",
	.parent_names = meson8b_vpu_0_1_parent_names,
	.num_parents = ARRAY_SIZE(meson8b_vpu_0_1_parent_names),
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VPU_CLK_CNTL,
		.mask = 0x3,
		.shift = 25,
	},
};

static struct clk_common_data meson8m2_vpu_1_sel = {
	.name = "vpu_1_sel",
	.parent_names = meson8m2_vpu_0_1_parent_names,
	.num_parents = ARRAY_SIZE(meson8m2_vpu_0_1_parent_names),
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VPU_CLK_CNTL,
		.mask = 0x3,
		.shift = 25,
	},
};

static struct clk_common_data meson8b_vpu_1_div = {
	.name = "vpu_1_div",
	.parent_name = "vpu_1_sel",
	/*
	 * Note:
	 * meson8b and meson8m2 have different vpu_1_sels (with
	 * different struct clk_hw). We fallback to the global
	 * naming string mechanism so vpu_1_div picks up the
	 * appropriate one.
	 */
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset = HHI_VPU_CLK_CNTL,
		.shift = 16,
		.width = 7,
	},
};

static struct clk_common_data meson8b_vpu_1 = {
	.name = "vpu_1",
	.parent_name = "vpu_1_div",
	.flags = CLK_SET_RATE_GATE | CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VPU_CLK_CNTL,
		.bit_idx = 24,
	},
};

/*
 * The VPU clock has two identical clock trees (vpu_0 and vpu_1)
 * muxed by a glitch-free switch on Meson8b and Meson8m2. The CCF can
 * actually manage this glitch-free mux because it does top-to-bottom
 * updates the each clock tree and switches to the "inactive" one when
 * CLK_SET_RATE_GATE is set.
 * Meson8 only has vpu_0 and no glitch-free mux.
 */
static struct clk_common_data meson8b_vpu = {
	.name = "vpu",
	.parent_names = (const char *const []) {
		"vpu_0",
		"vpu_1",
	},
	.num_parents = 2,
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VPU_CLK_CNTL,
		.mask = 1,
		.shift = 31,
	},
};

static const char *meson8b_vdec_parent_names[] = {
	"fclk_div4",
	"fclk_div3",
	"fclk_div5",
	"fclk_div7",
	"mpll2",
	"mpll1",
};

static struct clk_common_data meson8b_vdec_1_sel = {
	.name = "vdec_1_sel",
	.parent_names = meson8b_vdec_parent_names,
	.num_parents = ARRAY_SIZE(meson8b_vdec_parent_names),
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VDEC_CLK_CNTL,
		.mask = 0x3,
		.shift = 9,
		.flags = CLK_MUX_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_vdec_1_1_div = {
	.name = "vdec_1_1_div",
	.parent_name = "vdec_1_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset = HHI_VDEC_CLK_CNTL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_vdec_1_1 = {
	.name = "vdec_1_1",
	.parent_name = "vdec_1_1_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VDEC_CLK_CNTL,
		.bit_idx = 8,
	},
};

static struct clk_common_data meson8b_vdec_1_2_div = {
	.name = "vdec_1_2_div",
	.parent_name = "vdec_1_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset = HHI_VDEC3_CLK_CNTL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_vdec_1_2 = {
	.name = "vdec_1_2",
	.parent_name = "vdec_1_2_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VDEC3_CLK_CNTL,
		.bit_idx = 8,
	},
};

static struct clk_common_data meson8b_vdec_1 = {
	.name = "vdec_1",
	.parent_names = (const char *const []) {
		"vdec_1_1",
		"vdec_1_2",
	},
	.num_parents = 2,
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VDEC3_CLK_CNTL,
		.mask = 0x1,
		.shift = 15,
		.flags = CLK_MUX_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_vdec_hcodec_sel = {
	.name = "vdec_hcodec_sel",
	.parent_names = meson8b_vdec_parent_names,
	.num_parents = ARRAY_SIZE(meson8b_vdec_parent_names),
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VDEC_CLK_CNTL,
		.mask = 0x3,
		.shift = 25,
		.flags = CLK_MUX_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_vdec_hcodec_div = {
	.name = "vdec_hcodec_div",
	.parent_name = "vdec_hcodec_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset = HHI_VDEC_CLK_CNTL,
		.shift = 16,
		.width = 7,
		.flags = CLK_DIVIDER_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_vdec_hcodec = {
	.name = "vdec_hcodec",
	.parent_name = "vdec_hcodec_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VDEC_CLK_CNTL,
		.bit_idx = 24,
	},
};

static struct clk_common_data meson8b_vdec_2_sel = {
	.name = "vdec_2_sel",
	.parent_names = meson8b_vdec_parent_names,
	.num_parents = ARRAY_SIZE(meson8b_vdec_parent_names),
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VDEC2_CLK_CNTL,
		.mask = 0x3,
		.shift = 9,
		.flags = CLK_MUX_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_vdec_2_div = {
	.name = "vdec_2_div",
	.parent_name = "vdec_2_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset = HHI_VDEC2_CLK_CNTL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_vdec_2 = {
	.name = "vdec_2",
	.parent_name = "vdec_2_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VDEC2_CLK_CNTL,
		.bit_idx = 8,
	},
};

static struct clk_common_data meson8b_vdec_hevc_sel = {
	.name = "vdec_hevc_sel",
	.parent_names = meson8b_vdec_parent_names,
	.num_parents = ARRAY_SIZE(meson8b_vdec_parent_names),
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_MUX,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VDEC2_CLK_CNTL,
		.mask = 0x3,
		.shift = 25,
		.flags = CLK_MUX_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_vdec_hevc_div = {
	.name = "vdec_hevc_div",
	.parent_name = "vdec_hevc_sel",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_DIVIDER,
	.data = &(struct clk_regmap_div_data){
		.offset = HHI_VDEC2_CLK_CNTL,
		.shift = 16,
		.width = 7,
		.flags = CLK_DIVIDER_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_vdec_hevc_en = {
	.name = "vdec_hevc_en",
	.parent_name = "vdec_hevc_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_VDEC2_CLK_CNTL,
		.bit_idx = 24,
	},
};

static struct clk_common_data meson8b_vdec_hevc = {
	.name = "vdec_hevc",
	/* TODO: The second parent is currently unknown */
	.parent_names = (const char *const []) { "vdec_hevc_en" },
	.num_parents = 1,
	.flags = CLK_SET_RATE_PARENT,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_VDEC2_CLK_CNTL,
		.mask = 0x1,
		.shift = 31,
		.flags = CLK_MUX_ROUND_CLOSEST,
	},
};

/* TODO: the clock at index 0 is "DDR_PLL" which we don't support yet */
static const char *meson8b_cts_amclk_parent_names[] = {
	"mpll0",
	"mpll1",
	"mpll2"
};

static u32 meson8b_cts_amclk_mux_table[] = { 1, 2, 3 };

static struct clk_common_data meson8b_cts_amclk_sel = {
	.name = "cts_amclk_sel",
	.parent_names = meson8b_cts_amclk_parent_names,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_AUD_CLK_CNTL,
		.mask = 0x3,
		.shift = 9,
		.table = meson8b_cts_amclk_mux_table,
		.flags = CLK_MUX_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_cts_amclk_div = {
	.name = "cts_amclk_div",
	.parent_name = "cts_amclk_sel",
	.flags = CLK_SET_RATE_PARENT,
	.data = &(struct clk_regmap_div_data) {
		.offset = HHI_AUD_CLK_CNTL,
		.shift = 0,
		.width = 8,
		.flags = CLK_DIVIDER_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_cts_amclk = {
	.name = "cts_amclk",
	.parent_name = "cts_amclk_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_AUD_CLK_CNTL,
		.bit_idx = 8,
	},
};

/* TODO: the clock at index 0 is "DDR_PLL" which we don't support yet */
static const char *meson8b_cts_mclk_i958_parent_names[] = {
	"mpll0",
	"mpll1",
	"mpll2"
};

static u32 meson8b_cts_mclk_i958_mux_table[] = { 1, 2, 3 };

static struct clk_common_data meson8b_cts_mclk_i958_sel = {
	.name = "cts_mclk_i958_sel",
	.parent_names = meson8b_cts_mclk_i958_parent_names,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_AUD_CLK_CNTL2,
		.mask = 0x3,
		.shift = 25,
		.table = meson8b_cts_mclk_i958_mux_table,
		.flags = CLK_MUX_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_cts_mclk_i958_div = {
	.name = "cts_mclk_i958_div",
	.parent_name = "cts_mclk_i958_sel",
	.flags = CLK_SET_RATE_PARENT,
	.data = &(struct clk_regmap_div_data){
		.offset = HHI_AUD_CLK_CNTL2,
		.shift = 16,
		.width = 8,
		.flags = CLK_DIVIDER_ROUND_CLOSEST,
	},
};

static struct clk_common_data meson8b_cts_mclk_i958 = {
	.name = "cts_mclk_i958",
	.parent_name = "cts_mclk_i958_div",
	.flags = CLK_SET_RATE_PARENT,
	.type = CLK_GATE,
	.data = &(struct clk_regmap_gate_data){
		.offset = HHI_AUD_CLK_CNTL2,
		.bit_idx = 24,
	},
};

static struct clk_common_data meson8b_cts_i958 = {
	.name = "cts_i958",
	.parent_names = (const char *[]) {
		"cts_amclk",
		"cts_mclk_i958"
	},
	.num_parents = 2,
	/*
	 * The parent is specific to origin of the audio data. Let the
	 * consumer choose the appropriate parent.
	 */
	.flags = CLK_SET_RATE_PARENT | CLK_SET_RATE_NO_REPARENT,
	.data = &(struct clk_regmap_mux_data){
		.offset = HHI_AUD_CLK_CNTL2,
		.mask = 0x1,
		.shift = 27,
	},
};

#define MESON_GATE(_name, _reg, _bit) \
	MESON_PCLK(_name, _reg, _bit, clk81)

/* Everything Else (EE) domain gates */

static MESON_GATE(meson8b_ddr, HHI_GCLK_MPEG0, 0);
static MESON_GATE(meson8b_dos, HHI_GCLK_MPEG0, 1);
static MESON_GATE(meson8b_isa, HHI_GCLK_MPEG0, 5);
static MESON_GATE(meson8b_pl301, HHI_GCLK_MPEG0, 6);
static MESON_GATE(meson8b_periphs, HHI_GCLK_MPEG0, 7);
static MESON_GATE(meson8b_spicc, HHI_GCLK_MPEG0, 8);
static MESON_GATE(meson8b_i2c, HHI_GCLK_MPEG0, 9);
static MESON_GATE(meson8b_sar_adc, HHI_GCLK_MPEG0, 10);
static MESON_GATE(meson8b_smart_card, HHI_GCLK_MPEG0, 11);
static MESON_GATE(meson8b_rng0, HHI_GCLK_MPEG0, 12);
static MESON_GATE(meson8b_uart0, HHI_GCLK_MPEG0, 13);
static MESON_GATE(meson8b_sdhc, HHI_GCLK_MPEG0, 14);
static MESON_GATE(meson8b_stream, HHI_GCLK_MPEG0, 15);
static MESON_GATE(meson8b_async_fifo, HHI_GCLK_MPEG0, 16);
static MESON_GATE(meson8b_sdio, HHI_GCLK_MPEG0, 17);
static MESON_GATE(meson8b_abuf, HHI_GCLK_MPEG0, 18);
static MESON_GATE(meson8b_hiu_iface, HHI_GCLK_MPEG0, 19);
static MESON_GATE(meson8b_assist_misc, HHI_GCLK_MPEG0, 23);
static MESON_GATE(meson8b_spi, HHI_GCLK_MPEG0, 30);

static MESON_GATE(meson8b_i2s_spdif, HHI_GCLK_MPEG1, 2);
static MESON_GATE(meson8b_eth, HHI_GCLK_MPEG1, 3);
static MESON_GATE(meson8b_demux, HHI_GCLK_MPEG1, 4);
static MESON_GATE(meson8b_blkmv, HHI_GCLK_MPEG1, 14);
static MESON_GATE(meson8b_aiu, HHI_GCLK_MPEG1, 15);
static MESON_GATE(meson8b_uart1, HHI_GCLK_MPEG1, 16);
static MESON_GATE(meson8b_g2d, HHI_GCLK_MPEG1, 20);
static MESON_GATE(meson8b_usb0, HHI_GCLK_MPEG1, 21);
static MESON_GATE(meson8b_usb1, HHI_GCLK_MPEG1, 22);
static MESON_GATE(meson8b_reset, HHI_GCLK_MPEG1, 23);
static MESON_GATE(meson8b_nand, HHI_GCLK_MPEG1, 24);
static MESON_GATE(meson8b_dos_parser, HHI_GCLK_MPEG1, 25);
static MESON_GATE(meson8b_usb, HHI_GCLK_MPEG1, 26);
static MESON_GATE(meson8b_vdin1, HHI_GCLK_MPEG1, 28);
static MESON_GATE(meson8b_ahb_arb0, HHI_GCLK_MPEG1, 29);
static MESON_GATE(meson8b_efuse, HHI_GCLK_MPEG1, 30);
static MESON_GATE(meson8b_boot_rom, HHI_GCLK_MPEG1, 31);

static MESON_GATE(meson8b_ahb_data_bus, HHI_GCLK_MPEG2, 1);
static MESON_GATE(meson8b_ahb_ctrl_bus, HHI_GCLK_MPEG2, 2);
static MESON_GATE(meson8b_hdmi_intr_sync, HHI_GCLK_MPEG2, 3);
static MESON_GATE(meson8b_hdmi_pclk, HHI_GCLK_MPEG2, 4);
static MESON_GATE(meson8b_usb1_ddr_bridge, HHI_GCLK_MPEG2, 8);
static MESON_GATE(meson8b_usb0_ddr_bridge, HHI_GCLK_MPEG2, 9);
static MESON_GATE(meson8b_mmc_pclk, HHI_GCLK_MPEG2, 11);
static MESON_GATE(meson8b_dvin, HHI_GCLK_MPEG2, 12);
static MESON_GATE(meson8b_uart2, HHI_GCLK_MPEG2, 15);
static MESON_GATE(meson8b_sana, HHI_GCLK_MPEG2, 22);
static MESON_GATE(meson8b_vpu_intr, HHI_GCLK_MPEG2, 25);
static MESON_GATE(meson8b_sec_ahb_ahb3_bridge, HHI_GCLK_MPEG2, 26);
static MESON_GATE(meson8b_clk81_a9, HHI_GCLK_MPEG2, 29);

static MESON_GATE(meson8b_vclk2_venci0, HHI_GCLK_OTHER, 1);
static MESON_GATE(meson8b_vclk2_venci1, HHI_GCLK_OTHER, 2);
static MESON_GATE(meson8b_vclk2_vencp0, HHI_GCLK_OTHER, 3);
static MESON_GATE(meson8b_vclk2_vencp1, HHI_GCLK_OTHER, 4);
static MESON_GATE(meson8b_gclk_venci_int, HHI_GCLK_OTHER, 8);
static MESON_GATE(meson8b_gclk_vencp_int, HHI_GCLK_OTHER, 9);
static MESON_GATE(meson8b_dac_clk, HHI_GCLK_OTHER, 10);
static MESON_GATE(meson8b_aoclk_gate, HHI_GCLK_OTHER, 14);
static MESON_GATE(meson8b_iec958_gate, HHI_GCLK_OTHER, 16);
static MESON_GATE(meson8b_enc480p, HHI_GCLK_OTHER, 20);
static MESON_GATE(meson8b_rng1, HHI_GCLK_OTHER, 21);
static MESON_GATE(meson8b_gclk_vencl_int, HHI_GCLK_OTHER, 22);
static MESON_GATE(meson8b_vclk2_venclmcc, HHI_GCLK_OTHER, 24);
static MESON_GATE(meson8b_vclk2_vencl, HHI_GCLK_OTHER, 25);
static MESON_GATE(meson8b_vclk2_other, HHI_GCLK_OTHER, 26);
static MESON_GATE(meson8b_edp, HHI_GCLK_OTHER, 31);

/* AIU gates */
#define MESON_AIU_GLUE_GATE(_name, _reg, _bit) \
	MESON_PCLK(_name, _reg, _bit, meson8b_aiu_glue)

static MESON_PCLK(meson8b_aiu_glue, HHI_GCLK_MPEG1, 6, meson8b_aiu);
static MESON_AIU_GLUE_GATE(meson8b_iec958, HHI_GCLK_MPEG1, 7);
static MESON_AIU_GLUE_GATE(meson8b_i2s_out, HHI_GCLK_MPEG1, 8);
static MESON_AIU_GLUE_GATE(meson8b_amclk, HHI_GCLK_MPEG1, 9);
static MESON_AIU_GLUE_GATE(meson8b_aififo2, HHI_GCLK_MPEG1, 10);
static MESON_AIU_GLUE_GATE(meson8b_mixer, HHI_GCLK_MPEG1, 11);
static MESON_AIU_GLUE_GATE(meson8b_mixer_iface, HHI_GCLK_MPEG1, 12);
static MESON_AIU_GLUE_GATE(meson8b_adc, HHI_GCLK_MPEG1, 13);

/* Always On (AO) domain gates */

static MESON_GATE(meson8b_ao_media_cpu, HHI_GCLK_AO, 0);
static MESON_GATE(meson8b_ao_ahb_sram, HHI_GCLK_AO, 1);
static MESON_GATE(meson8b_ao_ahb_bus, HHI_GCLK_AO, 2);
static MESON_GATE(meson8b_ao_iface, HHI_GCLK_AO, 3);

#if 0
static struct clk_hw *meson8_hw_clks[] = {
	[CLKID_PLL_FIXED] = &meson8b_fixed_pll.hw,
	[CLKID_PLL_VID] = &meson8b_vid_pll.hw,
	[CLKID_PLL_SYS] = &meson8b_sys_pll.hw,
	[CLKID_FCLK_DIV2] = &meson8b_fclk_div2.hw,
	[CLKID_FCLK_DIV3] = &meson8b_fclk_div3.hw,
	[CLKID_FCLK_DIV4] = &meson8b_fclk_div4.hw,
	[CLKID_FCLK_DIV5] = &meson8b_fclk_div5.hw,
	[CLKID_FCLK_DIV7] = &meson8b_fclk_div7.hw,
	[CLKID_CPUCLK] = &meson8b_cpu_clk.hw,
	[CLKID_MPEG_SEL] = &meson8b_mpeg_clk_sel.hw,
	[CLKID_MPEG_DIV] = &meson8b_mpeg_clk_div.hw,
	[CLKID_CLK81] = &meson8b_clk81.hw,
	[CLKID_DDR]		    = &meson8b_ddr.hw,
	[CLKID_DOS]		    = &meson8b_dos.hw,
	[CLKID_ISA]		    = &meson8b_isa.hw,
	[CLKID_PL301]		    = &meson8b_pl301.hw,
	[CLKID_PERIPHS]		    = &meson8b_periphs.hw,
	[CLKID_SPICC]		    = &meson8b_spicc.hw,
	[CLKID_I2C]		    = &meson8b_i2c.hw,
	[CLKID_SAR_ADC]		    = &meson8b_sar_adc.hw,
	[CLKID_SMART_CARD]	    = &meson8b_smart_card.hw,
	[CLKID_RNG0]		    = &meson8b_rng0.hw,
	[CLKID_UART0]		    = &meson8b_uart0.hw,
	[CLKID_SDHC]		    = &meson8b_sdhc.hw,
	[CLKID_STREAM]		    = &meson8b_stream.hw,
	[CLKID_ASYNC_FIFO]	    = &meson8b_async_fifo.hw,
	[CLKID_SDIO]		    = &meson8b_sdio.hw,
	[CLKID_ABUF]		    = &meson8b_abuf.hw,
	[CLKID_HIU_IFACE]	    = &meson8b_hiu_iface.hw,
	[CLKID_ASSIST_MISC]	    = &meson8b_assist_misc.hw,
	[CLKID_SPI]		    = &meson8b_spi.hw,
	[CLKID_I2S_SPDIF]	    = &meson8b_i2s_spdif.hw,
	[CLKID_ETH]		    = &meson8b_eth.hw,
	[CLKID_DEMUX]		    = &meson8b_demux.hw,
	[CLKID_AIU_GLUE]	    = &meson8b_aiu_glue.hw,
	[CLKID_IEC958]		    = &meson8b_iec958.hw,
	[CLKID_I2S_OUT]		    = &meson8b_i2s_out.hw,
	[CLKID_AMCLK]		    = &meson8b_amclk.hw,
	[CLKID_AIFIFO2]		    = &meson8b_aififo2.hw,
	[CLKID_MIXER]		    = &meson8b_mixer.hw,
	[CLKID_MIXER_IFACE]	    = &meson8b_mixer_iface.hw,
	[CLKID_ADC]		    = &meson8b_adc.hw,
	[CLKID_BLKMV]		    = &meson8b_blkmv.hw,
	[CLKID_AIU]		    = &meson8b_aiu.hw,
	[CLKID_UART1]		    = &meson8b_uart1.hw,
	[CLKID_G2D]		    = &meson8b_g2d.hw,
	[CLKID_USB0]		    = &meson8b_usb0.hw,
	[CLKID_USB1]		    = &meson8b_usb1.hw,
	[CLKID_RESET]		    = &meson8b_reset.hw,
	[CLKID_NAND]		    = &meson8b_nand.hw,
	[CLKID_DOS_PARSER]	    = &meson8b_dos_parser.hw,
	[CLKID_USB]		    = &meson8b_usb.hw,
	[CLKID_VDIN1]		    = &meson8b_vdin1.hw,
	[CLKID_AHB_ARB0]	    = &meson8b_ahb_arb0.hw,
	[CLKID_EFUSE]		    = &meson8b_efuse.hw,
	[CLKID_BOOT_ROM]	    = &meson8b_boot_rom.hw,
	[CLKID_AHB_DATA_BUS]	    = &meson8b_ahb_data_bus.hw,
	[CLKID_AHB_CTRL_BUS]	    = &meson8b_ahb_ctrl_bus.hw,
	[CLKID_HDMI_INTR_SYNC]	    = &meson8b_hdmi_intr_sync.hw,
	[CLKID_HDMI_PCLK]	    = &meson8b_hdmi_pclk.hw,
	[CLKID_USB1_DDR_BRIDGE]	    = &meson8b_usb1_ddr_bridge.hw,
	[CLKID_USB0_DDR_BRIDGE]	    = &meson8b_usb0_ddr_bridge.hw,
	[CLKID_MMC_PCLK]	    = &meson8b_mmc_pclk.hw,
	[CLKID_DVIN]		    = &meson8b_dvin.hw,
	[CLKID_UART2]		    = &meson8b_uart2.hw,
	[CLKID_SANA]		    = &meson8b_sana.hw,
	[CLKID_VPU_INTR]	    = &meson8b_vpu_intr.hw,
	[CLKID_SEC_AHB_AHB3_BRIDGE] = &meson8b_sec_ahb_ahb3_bridge.hw,
	[CLKID_CLK81_A9]	    = &meson8b_clk81_a9.hw,
	[CLKID_VCLK2_VENCI0]	    = &meson8b_vclk2_venci0.hw,
	[CLKID_VCLK2_VENCI1]	    = &meson8b_vclk2_venci1.hw,
	[CLKID_VCLK2_VENCP0]	    = &meson8b_vclk2_vencp0.hw,
	[CLKID_VCLK2_VENCP1]	    = &meson8b_vclk2_vencp1.hw,
	[CLKID_GCLK_VENCI_INT]	    = &meson8b_gclk_venci_int.hw,
	[CLKID_GCLK_VENCP_INT]	    = &meson8b_gclk_vencp_int.hw,
	[CLKID_DAC_CLK]		    = &meson8b_dac_clk.hw,
	[CLKID_AOCLK_GATE]	    = &meson8b_aoclk_gate.hw,
	[CLKID_IEC958_GATE]	    = &meson8b_iec958_gate.hw,
	[CLKID_ENC480P]		    = &meson8b_enc480p.hw,
	[CLKID_RNG1]		    = &meson8b_rng1.hw,
	[CLKID_GCLK_VENCL_INT]	    = &meson8b_gclk_vencl_int.hw,
	[CLKID_VCLK2_VENCLMCC]	    = &meson8b_vclk2_venclmcc.hw,
	[CLKID_VCLK2_VENCL]	    = &meson8b_vclk2_vencl.hw,
	[CLKID_VCLK2_OTHER]	    = &meson8b_vclk2_other.hw,
	[CLKID_EDP]		    = &meson8b_edp.hw,
	[CLKID_AO_MEDIA_CPU]	    = &meson8b_ao_media_cpu.hw,
	[CLKID_AO_AHB_SRAM]	    = &meson8b_ao_ahb_sram.hw,
	[CLKID_AO_AHB_BUS]	    = &meson8b_ao_ahb_bus.hw,
	[CLKID_AO_IFACE]	    = &meson8b_ao_iface.hw,
	[CLKID_MPLL0]		    = &meson8b_mpll0.hw,
	[CLKID_MPLL1]		    = &meson8b_mpll1.hw,
	[CLKID_MPLL2]		    = &meson8b_mpll2.hw,
	[CLKID_MPLL0_DIV]	    = &meson8b_mpll0_div.hw,
	[CLKID_MPLL1_DIV]	    = &meson8b_mpll1_div.hw,
	[CLKID_MPLL2_DIV]	    = &meson8b_mpll2_div.hw,
	[CLKID_CPU_IN_SEL]	    = &meson8b_cpu_in_sel.hw,
	[CLKID_CPU_IN_DIV2]	    = &meson8b_cpu_in_div2.hw,
	[CLKID_CPU_IN_DIV3]	    = &meson8b_cpu_in_div3.hw,
	[CLKID_CPU_SCALE_DIV]	    = &meson8b_cpu_scale_div.hw,
	[CLKID_CPU_SCALE_OUT_SEL]   = &meson8b_cpu_scale_out_sel.hw,
	[CLKID_MPLL_PREDIV]	    = &meson8b_mpll_prediv.hw,
	[CLKID_FCLK_DIV2_DIV]	    = &meson8b_fclk_div2_div.hw,
	[CLKID_FCLK_DIV3_DIV]	    = &meson8b_fclk_div3_div.hw,
	[CLKID_FCLK_DIV4_DIV]	    = &meson8b_fclk_div4_div.hw,
	[CLKID_FCLK_DIV5_DIV]	    = &meson8b_fclk_div5_div.hw,
	[CLKID_FCLK_DIV7_DIV]	    = &meson8b_fclk_div7_div.hw,
	[CLKID_NAND_SEL]	    = &meson8b_nand_clk_sel.hw,
	[CLKID_NAND_DIV]	    = &meson8b_nand_clk_div.hw,
	[CLKID_NAND_CLK]	    = &meson8b_nand_clk_gate.hw,
	[CLKID_PLL_FIXED_DCO]	    = &meson8b_fixed_pll_dco.hw,
	[CLKID_HDMI_PLL_DCO]	    = &meson8b_hdmi_pll_dco.hw,
	[CLKID_PLL_SYS_DCO]	    = &meson8b_sys_pll_dco.hw,
	[CLKID_CPU_CLK_DIV2]	    = &meson8b_cpu_clk_div2.hw,
	[CLKID_CPU_CLK_DIV3]	    = &meson8b_cpu_clk_div3.hw,
	[CLKID_CPU_CLK_DIV4]	    = &meson8b_cpu_clk_div4.hw,
	[CLKID_CPU_CLK_DIV5]	    = &meson8b_cpu_clk_div5.hw,
	[CLKID_CPU_CLK_DIV6]	    = &meson8b_cpu_clk_div6.hw,
	[CLKID_CPU_CLK_DIV7]	    = &meson8b_cpu_clk_div7.hw,
	[CLKID_CPU_CLK_DIV8]	    = &meson8b_cpu_clk_div8.hw,
	[CLKID_APB_SEL]		    = &meson8b_apb_clk_sel.hw,
	[CLKID_APB]		    = &meson8b_apb_clk_gate.hw,
	[CLKID_PERIPH_SEL]	    = &meson8b_periph_clk_sel.hw,
	[CLKID_PERIPH]		    = &meson8b_periph_clk_gate.hw,
	[CLKID_AXI_SEL]		    = &meson8b_axi_clk_sel.hw,
	[CLKID_AXI]		    = &meson8b_axi_clk_gate.hw,
	[CLKID_L2_DRAM_SEL]	    = &meson8b_l2_dram_clk_sel.hw,
	[CLKID_L2_DRAM]		    = &meson8b_l2_dram_clk_gate.hw,
	[CLKID_HDMI_PLL_LVDS_OUT]   = &meson8b_hdmi_pll_lvds_out.hw,
	[CLKID_HDMI_PLL_HDMI_OUT]   = &meson8b_hdmi_pll_hdmi_out.hw,
	[CLKID_VID_PLL_IN_SEL]	    = &meson8b_vid_pll_in_sel.hw,
	[CLKID_VID_PLL_IN_EN]	    = &meson8b_vid_pll_in_en.hw,
	[CLKID_VID_PLL_PRE_DIV]	    = &meson8b_vid_pll_pre_div.hw,
	[CLKID_VID_PLL_POST_DIV]    = &meson8b_vid_pll_post_div.hw,
	[CLKID_VID_PLL_FINAL_DIV]   = &meson8b_vid_pll_final_div.hw,
	[CLKID_VCLK_IN_SEL]	    = &meson8b_vclk_in_sel.hw,
	[CLKID_VCLK_IN_EN]	    = &meson8b_vclk_in_en.hw,
	[CLKID_VCLK_EN]		    = &meson8b_vclk_en.hw,
	[CLKID_VCLK_DIV1]	    = &meson8b_vclk_div1_gate.hw,
	[CLKID_VCLK_DIV2_DIV]	    = &meson8b_vclk_div2_div.hw,
	[CLKID_VCLK_DIV2]	    = &meson8b_vclk_div2_div_gate.hw,
	[CLKID_VCLK_DIV4_DIV]	    = &meson8b_vclk_div4_div.hw,
	[CLKID_VCLK_DIV4]	    = &meson8b_vclk_div4_div_gate.hw,
	[CLKID_VCLK_DIV6_DIV]	    = &meson8b_vclk_div6_div.hw,
	[CLKID_VCLK_DIV6]	    = &meson8b_vclk_div6_div_gate.hw,
	[CLKID_VCLK_DIV12_DIV]	    = &meson8b_vclk_div12_div.hw,
	[CLKID_VCLK_DIV12]	    = &meson8b_vclk_div12_div_gate.hw,
	[CLKID_VCLK2_IN_SEL]	    = &meson8b_vclk2_in_sel.hw,
	[CLKID_VCLK2_IN_EN]	    = &meson8b_vclk2_clk_in_en.hw,
	[CLKID_VCLK2_EN]	    = &meson8b_vclk2_clk_en.hw,
	[CLKID_VCLK2_DIV1]	    = &meson8b_vclk2_div1_gate.hw,
	[CLKID_VCLK2_DIV2_DIV]	    = &meson8b_vclk2_div2_div.hw,
	[CLKID_VCLK2_DIV2]	    = &meson8b_vclk2_div2_div_gate.hw,
	[CLKID_VCLK2_DIV4_DIV]	    = &meson8b_vclk2_div4_div.hw,
	[CLKID_VCLK2_DIV4]	    = &meson8b_vclk2_div4_div_gate.hw,
	[CLKID_VCLK2_DIV6_DIV]	    = &meson8b_vclk2_div6_div.hw,
	[CLKID_VCLK2_DIV6]	    = &meson8b_vclk2_div6_div_gate.hw,
	[CLKID_VCLK2_DIV12_DIV]	    = &meson8b_vclk2_div12_div.hw,
	[CLKID_VCLK2_DIV12]	    = &meson8b_vclk2_div12_div_gate.hw,
	[CLKID_CTS_ENCT_SEL]	    = &meson8b_cts_enct_sel.hw,
	[CLKID_CTS_ENCT]	    = &meson8b_cts_enct.hw,
	[CLKID_CTS_ENCP_SEL]	    = &meson8b_cts_encp_sel.hw,
	[CLKID_CTS_ENCP]	    = &meson8b_cts_encp.hw,
	[CLKID_CTS_ENCI_SEL]	    = &meson8b_cts_enci_sel.hw,
	[CLKID_CTS_ENCI]	    = &meson8b_cts_enci.hw,
	[CLKID_HDMI_TX_PIXEL_SEL]   = &meson8b_hdmi_tx_pixel_sel.hw,
	[CLKID_HDMI_TX_PIXEL]	    = &meson8b_hdmi_tx_pixel.hw,
	[CLKID_CTS_ENCL_SEL]	    = &meson8b_cts_encl_sel.hw,
	[CLKID_CTS_ENCL]	    = &meson8b_cts_encl.hw,
	[CLKID_CTS_VDAC0_SEL]	    = &meson8b_cts_vdac0_sel.hw,
	[CLKID_CTS_VDAC0]	    = &meson8b_cts_vdac0.hw,
	[CLKID_HDMI_SYS_SEL]	    = &meson8b_hdmi_sys_sel.hw,
	[CLKID_HDMI_SYS_DIV]	    = &meson8b_hdmi_sys_div.hw,
	[CLKID_HDMI_SYS]	    = &meson8b_hdmi_sys.hw,
	[CLKID_MALI_0_SEL]	    = &meson8b_mali_0_sel.hw,
	[CLKID_MALI_0_DIV]	    = &meson8b_mali_0_div.hw,
	[CLKID_MALI]		    = &meson8b_mali_0.hw,
	[CLKID_VPU_0_SEL]	    = &meson8b_vpu_0_sel.hw,
	[CLKID_VPU_0_DIV]	    = &meson8b_vpu_0_div.hw,
	[CLKID_VPU]		    = &meson8b_vpu_0.hw,
	[CLKID_VDEC_1_SEL]	    = &meson8b_vdec_1_sel.hw,
	[CLKID_VDEC_1_1_DIV]	    = &meson8b_vdec_1_1_div.hw,
	[CLKID_VDEC_1]		    = &meson8b_vdec_1_1.hw,
	[CLKID_VDEC_HCODEC_SEL]	    = &meson8b_vdec_hcodec_sel.hw,
	[CLKID_VDEC_HCODEC_DIV]	    = &meson8b_vdec_hcodec_div.hw,
	[CLKID_VDEC_HCODEC]	    = &meson8b_vdec_hcodec.hw,
	[CLKID_VDEC_2_SEL]	    = &meson8b_vdec_2_sel.hw,
	[CLKID_VDEC_2_DIV]	    = &meson8b_vdec_2_div.hw,
	[CLKID_VDEC_2]		    = &meson8b_vdec_2.hw,
	[CLKID_VDEC_HEVC_SEL]	    = &meson8b_vdec_hevc_sel.hw,
	[CLKID_VDEC_HEVC_DIV]	    = &meson8b_vdec_hevc_div.hw,
	[CLKID_VDEC_HEVC_EN]	    = &meson8b_vdec_hevc_en.hw,
	[CLKID_VDEC_HEVC]	    = &meson8b_vdec_hevc.hw,
	[CLKID_CTS_AMCLK_SEL]	    = &meson8b_cts_amclk_sel.hw,
	[CLKID_CTS_AMCLK_DIV]	    = &meson8b_cts_amclk_div.hw,
	[CLKID_CTS_AMCLK]	    = &meson8b_cts_amclk.hw,
	[CLKID_CTS_MCLK_I958_SEL]   = &meson8b_cts_mclk_i958_sel.hw,
	[CLKID_CTS_MCLK_I958_DIV]   = &meson8b_cts_mclk_i958_div.hw,
	[CLKID_CTS_MCLK_I958]	    = &meson8b_cts_mclk_i958.hw,
	[CLKID_CTS_I958]	    = &meson8b_cts_i958.hw,
	[CLKID_VID_PLL_LVDS_EN]	    = &meson8b_vid_pll_lvds_en.hw,
	[CLKID_HDMI_PLL_DCO_IN]	    = &hdmi_pll_dco_in.hw,
};
#endif

static struct clk_common_data *meson8b_clks[] = {
	[CLKID_PLL_FIXED] = &meson8b_fixed_pll,
	[CLKID_PLL_VID] = &meson8b_vid_pll,
	[CLKID_PLL_SYS] = &meson8b_sys_pll,
	[CLKID_FCLK_DIV2] = &meson8b_fclk_div2,
	[CLKID_FCLK_DIV3] = &meson8b_fclk_div3,
	[CLKID_FCLK_DIV4] = &meson8b_fclk_div4,
	[CLKID_FCLK_DIV5] = &meson8b_fclk_div5,
	[CLKID_FCLK_DIV7] = &meson8b_fclk_div7,
	[CLKID_CPUCLK] = &meson8b_cpu_clk,
	[CLKID_MPEG_SEL] = &meson8b_mpeg_clk_sel,
	[CLKID_MPEG_DIV] = &meson8b_mpeg_clk_div,
	[CLKID_CLK81] = &meson8b_clk81,
	[CLKID_DDR]		    = &meson8b_ddr,
	[CLKID_DOS]		    = &meson8b_dos,
	[CLKID_ISA]		    = &meson8b_isa,
	[CLKID_PL301]		    = &meson8b_pl301,
	[CLKID_PERIPHS]		    = &meson8b_periphs,
	[CLKID_SPICC]		    = &meson8b_spicc,
	[CLKID_I2C]		    = &meson8b_i2c,
	[CLKID_SAR_ADC]		    = &meson8b_sar_adc,
	[CLKID_SMART_CARD]	    = &meson8b_smart_card,
	[CLKID_RNG0]		    = &meson8b_rng0,
	[CLKID_UART0]		    = &meson8b_uart0,
	[CLKID_SDHC]		    = &meson8b_sdhc,
	[CLKID_STREAM]		    = &meson8b_stream,
	[CLKID_ASYNC_FIFO]	    = &meson8b_async_fifo,
	[CLKID_SDIO]		    = &meson8b_sdio,
	[CLKID_ABUF]		    = &meson8b_abuf,
	[CLKID_HIU_IFACE]	    = &meson8b_hiu_iface,
	[CLKID_ASSIST_MISC]	    = &meson8b_assist_misc,
	[CLKID_SPI]		    = &meson8b_spi,
	[CLKID_I2S_SPDIF]	    = &meson8b_i2s_spdif,
	[CLKID_ETH]		    = &meson8b_eth,
	[CLKID_DEMUX]		    = &meson8b_demux,
	[CLKID_AIU_GLUE]	    = &meson8b_aiu_glue,
	[CLKID_IEC958]		    = &meson8b_iec958,
	[CLKID_I2S_OUT]		    = &meson8b_i2s_out,
	[CLKID_AMCLK]		    = &meson8b_amclk,
	[CLKID_AIFIFO2]		    = &meson8b_aififo2,
	[CLKID_MIXER]		    = &meson8b_mixer,
	[CLKID_MIXER_IFACE]	    = &meson8b_mixer_iface,
	[CLKID_ADC]		    = &meson8b_adc,
	[CLKID_BLKMV]		    = &meson8b_blkmv,
	[CLKID_AIU]		    = &meson8b_aiu,
	[CLKID_UART1]		    = &meson8b_uart1,
	[CLKID_G2D]		    = &meson8b_g2d,
	[CLKID_USB0]		    = &meson8b_usb0,
	[CLKID_USB1]		    = &meson8b_usb1,
	[CLKID_RESET]		    = &meson8b_reset,
	[CLKID_NAND]		    = &meson8b_nand,
	[CLKID_DOS_PARSER]	    = &meson8b_dos_parser,
	[CLKID_USB]		    = &meson8b_usb,
	[CLKID_VDIN1]		    = &meson8b_vdin1,
	[CLKID_AHB_ARB0]	    = &meson8b_ahb_arb0,
	[CLKID_EFUSE]		    = &meson8b_efuse,
	[CLKID_BOOT_ROM]	    = &meson8b_boot_rom,
	[CLKID_AHB_DATA_BUS]	    = &meson8b_ahb_data_bus,
	[CLKID_AHB_CTRL_BUS]	    = &meson8b_ahb_ctrl_bus,
	[CLKID_HDMI_INTR_SYNC]	    = &meson8b_hdmi_intr_sync,
	[CLKID_HDMI_PCLK]	    = &meson8b_hdmi_pclk,
	[CLKID_USB1_DDR_BRIDGE]	    = &meson8b_usb1_ddr_bridge,
	[CLKID_USB0_DDR_BRIDGE]	    = &meson8b_usb0_ddr_bridge,
	[CLKID_MMC_PCLK]	    = &meson8b_mmc_pclk,
	[CLKID_DVIN]		    = &meson8b_dvin,
	[CLKID_UART2]		    = &meson8b_uart2,
	[CLKID_SANA]		    = &meson8b_sana,
	[CLKID_VPU_INTR]	    = &meson8b_vpu_intr,
	[CLKID_SEC_AHB_AHB3_BRIDGE] = &meson8b_sec_ahb_ahb3_bridge,
	[CLKID_CLK81_A9]	    = &meson8b_clk81_a9,
	[CLKID_VCLK2_VENCI0]	    = &meson8b_vclk2_venci0,
	[CLKID_VCLK2_VENCI1]	    = &meson8b_vclk2_venci1,
	[CLKID_VCLK2_VENCP0]	    = &meson8b_vclk2_vencp0,
	[CLKID_VCLK2_VENCP1]	    = &meson8b_vclk2_vencp1,
	[CLKID_GCLK_VENCI_INT]	    = &meson8b_gclk_venci_int,
	[CLKID_GCLK_VENCP_INT]	    = &meson8b_gclk_vencp_int,
	[CLKID_DAC_CLK]		    = &meson8b_dac_clk,
	[CLKID_AOCLK_GATE]	    = &meson8b_aoclk_gate,
	[CLKID_IEC958_GATE]	    = &meson8b_iec958_gate,
	[CLKID_ENC480P]		    = &meson8b_enc480p,
	[CLKID_RNG1]		    = &meson8b_rng1,
	[CLKID_GCLK_VENCL_INT]	    = &meson8b_gclk_vencl_int,
	[CLKID_VCLK2_VENCLMCC]	    = &meson8b_vclk2_venclmcc,
	[CLKID_VCLK2_VENCL]	    = &meson8b_vclk2_vencl,
	[CLKID_VCLK2_OTHER]	    = &meson8b_vclk2_other,
	[CLKID_EDP]		    = &meson8b_edp,
	[CLKID_AO_MEDIA_CPU]	    = &meson8b_ao_media_cpu,
	[CLKID_AO_AHB_SRAM]	    = &meson8b_ao_ahb_sram,
	[CLKID_AO_AHB_BUS]	    = &meson8b_ao_ahb_bus,
	[CLKID_AO_IFACE]	    = &meson8b_ao_iface,
	[CLKID_MPLL0]		    = &meson8b_mpll0,
	[CLKID_MPLL1]		    = &meson8b_mpll1,
	[CLKID_MPLL2]		    = &meson8b_mpll2,
	//[CLKID_MPLL0_DIV]	    = &meson8b_mpll0_div,
	//[CLKID_MPLL1_DIV]	    = &meson8b_mpll1_div,
	//[CLKID_MPLL2_DIV]	    = &meson8b_mpll2_div,
	[CLKID_CPU_IN_SEL]	    = &meson8b_cpu_in_sel,
	[CLKID_CPU_IN_DIV2]	    = &meson8b_cpu_in_div2,
	[CLKID_CPU_IN_DIV3]	    = &meson8b_cpu_in_div3,
	[CLKID_CPU_SCALE_DIV]	    = &meson8b_cpu_scale_div,
	[CLKID_CPU_SCALE_OUT_SEL]   = &meson8b_cpu_scale_out_sel,
	[CLKID_MPLL_PREDIV]	    = &meson8b_mpll_prediv,
	[CLKID_FCLK_DIV2_DIV]	    = &meson8b_fclk_div2_div,
	[CLKID_FCLK_DIV3_DIV]	    = &meson8b_fclk_div3_div,
	[CLKID_FCLK_DIV4_DIV]	    = &meson8b_fclk_div4_div,
	[CLKID_FCLK_DIV5_DIV]	    = &meson8b_fclk_div5_div,
	[CLKID_FCLK_DIV7_DIV]	    = &meson8b_fclk_div7_div,
	[CLKID_NAND_SEL]	    = &meson8b_nand_clk_sel,
	[CLKID_NAND_DIV]	    = &meson8b_nand_clk_div,
	[CLKID_NAND_CLK]	    = &meson8b_nand_clk_gate,
	[CLKID_PLL_FIXED_DCO]	    = &meson8b_fixed_pll_dco,
	//[CLKID_HDMI_PLL_DCO]	    = &meson8b_hdmi_pll_dco,
	//[CLKID_PLL_SYS_DCO]	    = &meson8b_sys_pll_dco,
	[CLKID_CPU_CLK_DIV2]	    = &meson8b_cpu_clk_div2,
	[CLKID_CPU_CLK_DIV3]	    = &meson8b_cpu_clk_div3,
	[CLKID_CPU_CLK_DIV4]	    = &meson8b_cpu_clk_div4,
	[CLKID_CPU_CLK_DIV5]	    = &meson8b_cpu_clk_div5,
	[CLKID_CPU_CLK_DIV6]	    = &meson8b_cpu_clk_div6,
	[CLKID_CPU_CLK_DIV7]	    = &meson8b_cpu_clk_div7,
	[CLKID_CPU_CLK_DIV8]	    = &meson8b_cpu_clk_div8,
	[CLKID_APB_SEL]		    = &meson8b_apb_clk_sel,
	[CLKID_APB]		    = &meson8b_apb_clk_gate,
	[CLKID_PERIPH_SEL]	    = &meson8b_periph_clk_sel,
	[CLKID_PERIPH]		    = &meson8b_periph_clk_gate,
	[CLKID_AXI_SEL]		    = &meson8b_axi_clk_sel,
	[CLKID_AXI]		    = &meson8b_axi_clk_gate,
	[CLKID_L2_DRAM_SEL]	    = &meson8b_l2_dram_clk_sel,
	[CLKID_L2_DRAM]		    = &meson8b_l2_dram_clk_gate,
	[CLKID_HDMI_PLL_LVDS_OUT]   = &meson8b_hdmi_pll_lvds_out,
	[CLKID_HDMI_PLL_HDMI_OUT]   = &meson8b_hdmi_pll_hdmi_out,
	[CLKID_VID_PLL_IN_SEL]	    = &meson8b_vid_pll_in_sel,
	[CLKID_VID_PLL_IN_EN]	    = &meson8b_vid_pll_in_en,
	[CLKID_VID_PLL_PRE_DIV]	    = &meson8b_vid_pll_pre_div,
	[CLKID_VID_PLL_POST_DIV]    = &meson8b_vid_pll_post_div,
	[CLKID_VID_PLL_FINAL_DIV]   = &meson8b_vid_pll_final_div,
	[CLKID_VCLK_IN_SEL]	    = &meson8b_vclk_in_sel,
	[CLKID_VCLK_IN_EN]	    = &meson8b_vclk_in_en,
	[CLKID_VCLK_EN]		    = &meson8b_vclk_en,
	[CLKID_VCLK_DIV1]	    = &meson8b_vclk_div1_gate,
	[CLKID_VCLK_DIV2_DIV]	    = &meson8b_vclk_div2_div,
	[CLKID_VCLK_DIV2]	    = &meson8b_vclk_div2_div_gate,
	[CLKID_VCLK_DIV4_DIV]	    = &meson8b_vclk_div4_div,
	[CLKID_VCLK_DIV4]	    = &meson8b_vclk_div4_div_gate,
	[CLKID_VCLK_DIV6_DIV]	    = &meson8b_vclk_div6_div,
	[CLKID_VCLK_DIV6]	    = &meson8b_vclk_div6_div_gate,
	[CLKID_VCLK_DIV12_DIV]	    = &meson8b_vclk_div12_div,
	[CLKID_VCLK_DIV12]	    = &meson8b_vclk_div12_div_gate,
	[CLKID_VCLK2_IN_SEL]	    = &meson8b_vclk2_in_sel,
	[CLKID_VCLK2_IN_EN]	    = &meson8b_vclk2_clk_in_en,
	[CLKID_VCLK2_EN]	    = &meson8b_vclk2_clk_en,
	[CLKID_VCLK2_DIV1]	    = &meson8b_vclk2_div1_gate,
	[CLKID_VCLK2_DIV2_DIV]	    = &meson8b_vclk2_div2_div,
	[CLKID_VCLK2_DIV2]	    = &meson8b_vclk2_div2_div_gate,
	[CLKID_VCLK2_DIV4_DIV]	    = &meson8b_vclk2_div4_div,
	[CLKID_VCLK2_DIV4]	    = &meson8b_vclk2_div4_div_gate,
	[CLKID_VCLK2_DIV6_DIV]	    = &meson8b_vclk2_div6_div,
	[CLKID_VCLK2_DIV6]	    = &meson8b_vclk2_div6_div_gate,
	[CLKID_VCLK2_DIV12_DIV]	    = &meson8b_vclk2_div12_div,
	[CLKID_VCLK2_DIV12]	    = &meson8b_vclk2_div12_div_gate,
	[CLKID_CTS_ENCT_SEL]	    = &meson8b_cts_enct_sel,
	[CLKID_CTS_ENCT]	    = &meson8b_cts_enct,
	[CLKID_CTS_ENCP_SEL]	    = &meson8b_cts_encp_sel,
	[CLKID_CTS_ENCP]	    = &meson8b_cts_encp,
	[CLKID_CTS_ENCI_SEL]	    = &meson8b_cts_enci_sel,
	[CLKID_CTS_ENCI]	    = &meson8b_cts_enci,
	[CLKID_HDMI_TX_PIXEL_SEL]   = &meson8b_hdmi_tx_pixel_sel,
	[CLKID_HDMI_TX_PIXEL]	    = &meson8b_hdmi_tx_pixel,
	[CLKID_CTS_ENCL_SEL]	    = &meson8b_cts_encl_sel,
	[CLKID_CTS_ENCL]	    = &meson8b_cts_encl,
	[CLKID_CTS_VDAC0_SEL]	    = &meson8b_cts_vdac0_sel,
	[CLKID_CTS_VDAC0]	    = &meson8b_cts_vdac0,
	[CLKID_HDMI_SYS_SEL]	    = &meson8b_hdmi_sys_sel,
	[CLKID_HDMI_SYS_DIV]	    = &meson8b_hdmi_sys_div,
	[CLKID_HDMI_SYS]	    = &meson8b_hdmi_sys,
	[CLKID_MALI_0_SEL]	    = &meson8b_mali_0_sel,
	[CLKID_MALI_0_DIV]	    = &meson8b_mali_0_div,
	[CLKID_MALI_0]		    = &meson8b_mali_0,
	[CLKID_MALI_1_SEL]	    = &meson8b_mali_1_sel,
	[CLKID_MALI_1_DIV]	    = &meson8b_mali_1_div,
	[CLKID_MALI_1]		    = &meson8b_mali_1,
	[CLKID_MALI]		    = &meson8b_mali,
	[CLKID_VPU_0_SEL]	    = &meson8b_vpu_0_sel,
	[CLKID_VPU_0_DIV]	    = &meson8b_vpu_0_div,
	[CLKID_VPU_0]		    = &meson8b_vpu_0,
	[CLKID_VPU_1_SEL]	    = &meson8b_vpu_1_sel,
	[CLKID_VPU_1_DIV]	    = &meson8b_vpu_1_div,
	[CLKID_VPU_1]		    = &meson8b_vpu_1,
	[CLKID_VPU]		    = &meson8b_vpu,
	[CLKID_VDEC_1_SEL]	    = &meson8b_vdec_1_sel,
	[CLKID_VDEC_1_1_DIV]	    = &meson8b_vdec_1_1_div,
	[CLKID_VDEC_1_1]	    = &meson8b_vdec_1_1,
	[CLKID_VDEC_1_2_DIV]	    = &meson8b_vdec_1_2_div,
	[CLKID_VDEC_1_2]	    = &meson8b_vdec_1_2,
	[CLKID_VDEC_1]		    = &meson8b_vdec_1,
	[CLKID_VDEC_HCODEC_SEL]	    = &meson8b_vdec_hcodec_sel,
	[CLKID_VDEC_HCODEC_DIV]	    = &meson8b_vdec_hcodec_div,
	[CLKID_VDEC_HCODEC]	    = &meson8b_vdec_hcodec,
	[CLKID_VDEC_2_SEL]	    = &meson8b_vdec_2_sel,
	[CLKID_VDEC_2_DIV]	    = &meson8b_vdec_2_div,
	[CLKID_VDEC_2]		    = &meson8b_vdec_2,
	[CLKID_VDEC_HEVC_SEL]	    = &meson8b_vdec_hevc_sel,
	[CLKID_VDEC_HEVC_DIV]	    = &meson8b_vdec_hevc_div,
	[CLKID_VDEC_HEVC_EN]	    = &meson8b_vdec_hevc_en,
	[CLKID_VDEC_HEVC]	    = &meson8b_vdec_hevc,
	[CLKID_CTS_AMCLK_SEL]	    = &meson8b_cts_amclk_sel,
	[CLKID_CTS_AMCLK_DIV]	    = &meson8b_cts_amclk_div,
	[CLKID_CTS_AMCLK]	    = &meson8b_cts_amclk,
	[CLKID_CTS_MCLK_I958_SEL]   = &meson8b_cts_mclk_i958_sel,
	[CLKID_CTS_MCLK_I958_DIV]   = &meson8b_cts_mclk_i958_div,
	[CLKID_CTS_MCLK_I958]	    = &meson8b_cts_mclk_i958,
	[CLKID_CTS_I958]	    = &meson8b_cts_i958,
	[CLKID_VID_PLL_LVDS_EN]	    = &meson8b_vid_pll_lvds_en,
	[CLKID_HDMI_PLL_DCO_IN]	    = &hdmi_pll_dco_in,
};

#if 0
static struct clk_hw *meson8m2_hw_clks[] = {
	[CLKID_PLL_FIXED] = &meson8b_fixed_pll.hw,
	[CLKID_PLL_VID] = &meson8b_vid_pll.hw,
	[CLKID_PLL_SYS] = &meson8b_sys_pll.hw,
	[CLKID_FCLK_DIV2] = &meson8b_fclk_div2.hw,
	[CLKID_FCLK_DIV3] = &meson8b_fclk_div3.hw,
	[CLKID_FCLK_DIV4] = &meson8b_fclk_div4.hw,
	[CLKID_FCLK_DIV5] = &meson8b_fclk_div5.hw,
	[CLKID_FCLK_DIV7] = &meson8b_fclk_div7.hw,
	[CLKID_CPUCLK] = &meson8b_cpu_clk.hw,
	[CLKID_MPEG_SEL] = &meson8b_mpeg_clk_sel.hw,
	[CLKID_MPEG_DIV] = &meson8b_mpeg_clk_div.hw,
	[CLKID_CLK81] = &meson8b_clk81.hw,
	[CLKID_DDR]		    = &meson8b_ddr.hw,
	[CLKID_DOS]		    = &meson8b_dos.hw,
	[CLKID_ISA]		    = &meson8b_isa.hw,
	[CLKID_PL301]		    = &meson8b_pl301.hw,
	[CLKID_PERIPHS]		    = &meson8b_periphs.hw,
	[CLKID_SPICC]		    = &meson8b_spicc.hw,
	[CLKID_I2C]		    = &meson8b_i2c.hw,
	[CLKID_SAR_ADC]		    = &meson8b_sar_adc.hw,
	[CLKID_SMART_CARD]	    = &meson8b_smart_card.hw,
	[CLKID_RNG0]		    = &meson8b_rng0.hw,
	[CLKID_UART0]		    = &meson8b_uart0.hw,
	[CLKID_SDHC]		    = &meson8b_sdhc.hw,
	[CLKID_STREAM]		    = &meson8b_stream.hw,
	[CLKID_ASYNC_FIFO]	    = &meson8b_async_fifo.hw,
	[CLKID_SDIO]		    = &meson8b_sdio.hw,
	[CLKID_ABUF]		    = &meson8b_abuf.hw,
	[CLKID_HIU_IFACE]	    = &meson8b_hiu_iface.hw,
	[CLKID_ASSIST_MISC]	    = &meson8b_assist_misc.hw,
	[CLKID_SPI]		    = &meson8b_spi.hw,
	[CLKID_I2S_SPDIF]	    = &meson8b_i2s_spdif.hw,
	[CLKID_ETH]		    = &meson8b_eth.hw,
	[CLKID_DEMUX]		    = &meson8b_demux.hw,
	[CLKID_AIU_GLUE]	    = &meson8b_aiu_glue.hw,
	[CLKID_IEC958]		    = &meson8b_iec958.hw,
	[CLKID_I2S_OUT]		    = &meson8b_i2s_out.hw,
	[CLKID_AMCLK]		    = &meson8b_amclk.hw,
	[CLKID_AIFIFO2]		    = &meson8b_aififo2.hw,
	[CLKID_MIXER]		    = &meson8b_mixer.hw,
	[CLKID_MIXER_IFACE]	    = &meson8b_mixer_iface.hw,
	[CLKID_ADC]		    = &meson8b_adc.hw,
	[CLKID_BLKMV]		    = &meson8b_blkmv.hw,
	[CLKID_AIU]		    = &meson8b_aiu.hw,
	[CLKID_UART1]		    = &meson8b_uart1.hw,
	[CLKID_G2D]		    = &meson8b_g2d.hw,
	[CLKID_USB0]		    = &meson8b_usb0.hw,
	[CLKID_USB1]		    = &meson8b_usb1.hw,
	[CLKID_RESET]		    = &meson8b_reset.hw,
	[CLKID_NAND]		    = &meson8b_nand.hw,
	[CLKID_DOS_PARSER]	    = &meson8b_dos_parser.hw,
	[CLKID_USB]		    = &meson8b_usb.hw,
	[CLKID_VDIN1]		    = &meson8b_vdin1.hw,
	[CLKID_AHB_ARB0]	    = &meson8b_ahb_arb0.hw,
	[CLKID_EFUSE]		    = &meson8b_efuse.hw,
	[CLKID_BOOT_ROM]	    = &meson8b_boot_rom.hw,
	[CLKID_AHB_DATA_BUS]	    = &meson8b_ahb_data_bus.hw,
	[CLKID_AHB_CTRL_BUS]	    = &meson8b_ahb_ctrl_bus.hw,
	[CLKID_HDMI_INTR_SYNC]	    = &meson8b_hdmi_intr_sync.hw,
		/*
		 * Note:
		 * meson8b and meson8m2 have different vpu_1_sels (with
		 * different struct clk_hw). We fallback to the global
		 * naming string mechanism so vpu_1_div picks up the
		 * appropriate one.
		 */
	[CLKID_HDMI_PCLK]	    = &meson8b_hdmi_pclk.hw,
	[CLKID_USB1_DDR_BRIDGE]	    = &meson8b_usb1_ddr_bridge.hw,
	[CLKID_USB0_DDR_BRIDGE]	    = &meson8b_usb0_ddr_bridge.hw,
	[CLKID_MMC_PCLK]	    = &meson8b_mmc_pclk.hw,
	[CLKID_DVIN]		    = &meson8b_dvin.hw,
	[CLKID_UART2]		    = &meson8b_uart2.hw,
	[CLKID_SANA]		    = &meson8b_sana.hw,
	[CLKID_VPU_INTR]	    = &meson8b_vpu_intr.hw,
	[CLKID_SEC_AHB_AHB3_BRIDGE] = &meson8b_sec_ahb_ahb3_bridge.hw,
	[CLKID_CLK81_A9]	    = &meson8b_clk81_a9.hw,
	[CLKID_VCLK2_VENCI0]	    = &meson8b_vclk2_venci0.hw,
	[CLKID_VCLK2_VENCI1]	    = &meson8b_vclk2_venci1.hw,
	[CLKID_VCLK2_VENCP0]	    = &meson8b_vclk2_vencp0.hw,
	[CLKID_VCLK2_VENCP1]	    = &meson8b_vclk2_vencp1.hw,
	[CLKID_GCLK_VENCI_INT]	    = &meson8b_gclk_venci_int.hw,
	[CLKID_GCLK_VENCP_INT]	    = &meson8b_gclk_vencp_int.hw,
	[CLKID_DAC_CLK]		    = &meson8b_dac_clk.hw,
	[CLKID_AOCLK_GATE]	    = &meson8b_aoclk_gate.hw,
	[CLKID_IEC958_GATE]	    = &meson8b_iec958_gate.hw,
	[CLKID_ENC480P]		    = &meson8b_enc480p.hw,
	[CLKID_RNG1]		    = &meson8b_rng1.hw,
	[CLKID_GCLK_VENCL_INT]	    = &meson8b_gclk_vencl_int.hw,
	[CLKID_VCLK2_VENCLMCC]	    = &meson8b_vclk2_venclmcc.hw,
	[CLKID_VCLK2_VENCL]	    = &meson8b_vclk2_vencl.hw,
	[CLKID_VCLK2_OTHER]	    = &meson8b_vclk2_other.hw,
	[CLKID_EDP]		    = &meson8b_edp.hw,
	[CLKID_AO_MEDIA_CPU]	    = &meson8b_ao_media_cpu.hw,
	[CLKID_AO_AHB_SRAM]	    = &meson8b_ao_ahb_sram.hw,
	[CLKID_AO_AHB_BUS]	    = &meson8b_ao_ahb_bus.hw,
	[CLKID_AO_IFACE]	    = &meson8b_ao_iface.hw,
	[CLKID_MPLL0]		    = &meson8b_mpll0.hw,
	[CLKID_MPLL1]		    = &meson8b_mpll1.hw,
	[CLKID_MPLL2]		    = &meson8b_mpll2.hw,
	[CLKID_MPLL0_DIV]	    = &meson8b_mpll0_div.hw,
	[CLKID_MPLL1_DIV]	    = &meson8b_mpll1_div.hw,
	[CLKID_MPLL2_DIV]	    = &meson8b_mpll2_div.hw,
	[CLKID_CPU_IN_SEL]	    = &meson8b_cpu_in_sel.hw,
	[CLKID_CPU_IN_DIV2]	    = &meson8b_cpu_in_div2.hw,
	[CLKID_CPU_IN_DIV3]	    = &meson8b_cpu_in_div3.hw,
	[CLKID_CPU_SCALE_DIV]	    = &meson8b_cpu_scale_div.hw,
	[CLKID_CPU_SCALE_OUT_SEL]   = &meson8b_cpu_scale_out_sel.hw,
	[CLKID_MPLL_PREDIV]	    = &meson8b_mpll_prediv.hw,
	[CLKID_FCLK_DIV2_DIV]	    = &meson8b_fclk_div2_div.hw,
	[CLKID_FCLK_DIV3_DIV]	    = &meson8b_fclk_div3_div.hw,
	[CLKID_FCLK_DIV4_DIV]	    = &meson8b_fclk_div4_div.hw,
	[CLKID_FCLK_DIV5_DIV]	    = &meson8b_fclk_div5_div.hw,
	[CLKID_FCLK_DIV7_DIV]	    = &meson8b_fclk_div7_div.hw,
	[CLKID_NAND_SEL]	    = &meson8b_nand_clk_sel.hw,
	[CLKID_NAND_DIV]	    = &meson8b_nand_clk_div.hw,
	[CLKID_NAND_CLK]	    = &meson8b_nand_clk_gate.hw,
	[CLKID_PLL_FIXED_DCO]	    = &meson8b_fixed_pll_dco.hw,
	[CLKID_HDMI_PLL_DCO]	    = &meson8b_hdmi_pll_dco.hw,
	[CLKID_PLL_SYS_DCO]	    = &meson8b_sys_pll_dco.hw,
	[CLKID_CPU_CLK_DIV2]	    = &meson8b_cpu_clk_div2.hw,
	[CLKID_CPU_CLK_DIV3]	    = &meson8b_cpu_clk_div3.hw,
	[CLKID_CPU_CLK_DIV4]	    = &meson8b_cpu_clk_div4.hw,
	[CLKID_CPU_CLK_DIV5]	    = &meson8b_cpu_clk_div5.hw,
	[CLKID_CPU_CLK_DIV6]	    = &meson8b_cpu_clk_div6.hw,
	[CLKID_CPU_CLK_DIV7]	    = &meson8b_cpu_clk_div7.hw,
	[CLKID_CPU_CLK_DIV8]	    = &meson8b_cpu_clk_div8.hw,
	[CLKID_APB_SEL]		    = &meson8b_apb_clk_sel.hw,
	[CLKID_APB]		    = &meson8b_apb_clk_gate.hw,
	[CLKID_PERIPH_SEL]	    = &meson8b_periph_clk_sel.hw,
	[CLKID_PERIPH]		    = &meson8b_periph_clk_gate.hw,
	[CLKID_AXI_SEL]		    = &meson8b_axi_clk_sel.hw,
	[CLKID_AXI]		    = &meson8b_axi_clk_gate.hw,
	[CLKID_L2_DRAM_SEL]	    = &meson8b_l2_dram_clk_sel.hw,
	[CLKID_L2_DRAM]		    = &meson8b_l2_dram_clk_gate.hw,
	[CLKID_HDMI_PLL_LVDS_OUT]   = &meson8b_hdmi_pll_lvds_out.hw,
	[CLKID_HDMI_PLL_HDMI_OUT]   = &meson8b_hdmi_pll_hdmi_out.hw,
	[CLKID_VID_PLL_IN_SEL]	    = &meson8b_vid_pll_in_sel.hw,
	[CLKID_VID_PLL_IN_EN]	    = &meson8b_vid_pll_in_en.hw,
	[CLKID_VID_PLL_PRE_DIV]	    = &meson8b_vid_pll_pre_div.hw,
	[CLKID_VID_PLL_POST_DIV]    = &meson8b_vid_pll_post_div.hw,
	[CLKID_VID_PLL_FINAL_DIV]   = &meson8b_vid_pll_final_div.hw,
	[CLKID_VCLK_IN_SEL]	    = &meson8b_vclk_in_sel.hw,
	[CLKID_VCLK_IN_EN]	    = &meson8b_vclk_in_en.hw,
	[CLKID_VCLK_EN]		    = &meson8b_vclk_en.hw,
	[CLKID_VCLK_DIV1]	    = &meson8b_vclk_div1_gate.hw,
	[CLKID_VCLK_DIV2_DIV]	    = &meson8b_vclk_div2_div.hw,
	[CLKID_VCLK_DIV2]	    = &meson8b_vclk_div2_div_gate.hw,
	[CLKID_VCLK_DIV4_DIV]	    = &meson8b_vclk_div4_div.hw,
	[CLKID_VCLK_DIV4]	    = &meson8b_vclk_div4_div_gate.hw,
	[CLKID_VCLK_DIV6_DIV]	    = &meson8b_vclk_div6_div.hw,
	[CLKID_VCLK_DIV6]	    = &meson8b_vclk_div6_div_gate.hw,
	[CLKID_VCLK_DIV12_DIV]	    = &meson8b_vclk_div12_div.hw,
	[CLKID_VCLK_DIV12]	    = &meson8b_vclk_div12_div_gate.hw,
	[CLKID_VCLK2_IN_SEL]	    = &meson8b_vclk2_in_sel.hw,
	[CLKID_VCLK2_IN_EN]	    = &meson8b_vclk2_clk_in_en.hw,
	[CLKID_VCLK2_EN]	    = &meson8b_vclk2_clk_en.hw,
	[CLKID_VCLK2_DIV1]	    = &meson8b_vclk2_div1_gate.hw,
	[CLKID_VCLK2_DIV2_DIV]	    = &meson8b_vclk2_div2_div.hw,
	[CLKID_VCLK2_DIV2]	    = &meson8b_vclk2_div2_div_gate.hw,
	[CLKID_VCLK2_DIV4_DIV]	    = &meson8b_vclk2_div4_div.hw,
	[CLKID_VCLK2_DIV4]	    = &meson8b_vclk2_div4_div_gate.hw,
	[CLKID_VCLK2_DIV6_DIV]	    = &meson8b_vclk2_div6_div.hw,
	[CLKID_VCLK2_DIV6]	    = &meson8b_vclk2_div6_div_gate.hw,
	[CLKID_VCLK2_DIV12_DIV]	    = &meson8b_vclk2_div12_div.hw,
	[CLKID_VCLK2_DIV12]	    = &meson8b_vclk2_div12_div_gate.hw,
	[CLKID_CTS_ENCT_SEL]	    = &meson8b_cts_enct_sel.hw,
	[CLKID_CTS_ENCT]	    = &meson8b_cts_enct.hw,
	[CLKID_CTS_ENCP_SEL]	    = &meson8b_cts_encp_sel.hw,
	[CLKID_CTS_ENCP]	    = &meson8b_cts_encp.hw,
	[CLKID_CTS_ENCI_SEL]	    = &meson8b_cts_enci_sel.hw,
	[CLKID_CTS_ENCI]	    = &meson8b_cts_enci.hw,
	[CLKID_HDMI_TX_PIXEL_SEL]   = &meson8b_hdmi_tx_pixel_sel.hw,
	[CLKID_HDMI_TX_PIXEL]	    = &meson8b_hdmi_tx_pixel.hw,
	[CLKID_CTS_ENCL_SEL]	    = &meson8b_cts_encl_sel.hw,
	[CLKID_CTS_ENCL]	    = &meson8b_cts_encl.hw,
	[CLKID_CTS_VDAC0_SEL]	    = &meson8b_cts_vdac0_sel.hw,
	[CLKID_CTS_VDAC0]	    = &meson8b_cts_vdac0.hw,
	[CLKID_HDMI_SYS_SEL]	    = &meson8b_hdmi_sys_sel.hw,
	[CLKID_HDMI_SYS_DIV]	    = &meson8b_hdmi_sys_div.hw,
	[CLKID_HDMI_SYS]	    = &meson8b_hdmi_sys.hw,
	[CLKID_MALI_0_SEL]	    = &meson8b_mali_0_sel.hw,
	[CLKID_MALI_0_DIV]	    = &meson8b_mali_0_div.hw,
	[CLKID_MALI_0]		    = &meson8b_mali_0.hw,
	[CLKID_MALI_1_SEL]	    = &meson8b_mali_1_sel.hw,
	[CLKID_MALI_1_DIV]	    = &meson8b_mali_1_div.hw,
	[CLKID_MALI_1]		    = &meson8b_mali_1.hw,
	[CLKID_MALI]		    = &meson8b_mali.hw,
	[CLKID_GP_PLL_DCO]	    = &meson8m2_gp_pll_dco.hw,
	[CLKID_GP_PLL]		    = &meson8m2_gp_pll.hw,
	[CLKID_VPU_0_SEL]	    = &meson8m2_vpu_0_sel.hw,
	[CLKID_VPU_0_DIV]	    = &meson8b_vpu_0_div.hw,
	[CLKID_VPU_0]		    = &meson8b_vpu_0.hw,
	[CLKID_VPU_1_SEL]	    = &meson8m2_vpu_1_sel.hw,
	[CLKID_VPU_1_DIV]	    = &meson8b_vpu_1_div.hw,
	[CLKID_VPU_1]		    = &meson8b_vpu_1.hw,
	[CLKID_VPU]		    = &meson8b_vpu.hw,
	[CLKID_VDEC_1_SEL]	    = &meson8b_vdec_1_sel.hw,
	[CLKID_VDEC_1_1_DIV]	    = &meson8b_vdec_1_1_div.hw,
	[CLKID_VDEC_1_1]	    = &meson8b_vdec_1_1.hw,
	[CLKID_VDEC_1_2_DIV]	    = &meson8b_vdec_1_2_div.hw,
	[CLKID_VDEC_1_2]	    = &meson8b_vdec_1_2.hw,
	[CLKID_VDEC_1]		    = &meson8b_vdec_1.hw,
	[CLKID_VDEC_HCODEC_SEL]	    = &meson8b_vdec_hcodec_sel.hw,
	[CLKID_VDEC_HCODEC_DIV]	    = &meson8b_vdec_hcodec_div.hw,
	[CLKID_VDEC_HCODEC]	    = &meson8b_vdec_hcodec.hw,
	[CLKID_VDEC_2_SEL]	    = &meson8b_vdec_2_sel.hw,
	[CLKID_VDEC_2_DIV]	    = &meson8b_vdec_2_div.hw,
	[CLKID_VDEC_2]		    = &meson8b_vdec_2.hw,
	[CLKID_VDEC_HEVC_SEL]	    = &meson8b_vdec_hevc_sel.hw,
	[CLKID_VDEC_HEVC_DIV]	    = &meson8b_vdec_hevc_div.hw,
	[CLKID_VDEC_HEVC_EN]	    = &meson8b_vdec_hevc_en.hw,
	[CLKID_VDEC_HEVC]	    = &meson8b_vdec_hevc.hw,
	[CLKID_CTS_AMCLK_SEL]	    = &meson8b_cts_amclk_sel.hw,
	[CLKID_CTS_AMCLK_DIV]	    = &meson8b_cts_amclk_div.hw,
	[CLKID_CTS_AMCLK]	    = &meson8b_cts_amclk.hw,
	[CLKID_CTS_MCLK_I958_SEL]   = &meson8b_cts_mclk_i958_sel.hw,
	[CLKID_CTS_MCLK_I958_DIV]   = &meson8b_cts_mclk_i958_div.hw,
	[CLKID_CTS_MCLK_I958]	    = &meson8b_cts_mclk_i958.hw,
	[CLKID_CTS_I958]	    = &meson8b_cts_i958.hw,
	[CLKID_VID_PLL_LVDS_EN]	    = &meson8b_vid_pll_lvds_en.hw,
	[CLKID_HDMI_PLL_DCO_IN]	    = &hdmi_pll_dco_in.hw,
};

static struct clk_common_data *const meson8b_clk_regmaps[] = {
	&meson8b_clk81,
	&meson8b_ddr,
	&meson8b_dos,
	&meson8b_isa,
	&meson8b_pl301,
	&meson8b_periphs,
	&meson8b_spicc,
	&meson8b_i2c,
	&meson8b_sar_adc,
	&meson8b_smart_card,
	&meson8b_rng0,
	&meson8b_uart0,
	&meson8b_sdhc,
	&meson8b_stream,
	&meson8b_async_fifo,
	&meson8b_sdio,
	&meson8b_abuf,
	&meson8b_hiu_iface,
	&meson8b_assist_misc,
	&meson8b_spi,
	&meson8b_i2s_spdif,
	&meson8b_eth,
	&meson8b_demux,
	&meson8b_aiu_glue,
	&meson8b_iec958,
	&meson8b_i2s_out,
	&meson8b_amclk,
	&meson8b_aififo2,
	&meson8b_mixer,
	&meson8b_mixer_iface,
	&meson8b_adc,
	&meson8b_blkmv,
	&meson8b_aiu,
	&meson8b_uart1,
	&meson8b_g2d,
	&meson8b_usb0,
	&meson8b_usb1,
	&meson8b_reset,
	&meson8b_nand,
	&meson8b_dos_parser,
	&meson8b_usb,
	&meson8b_vdin1,
	&meson8b_ahb_arb0,
	&meson8b_efuse,
	&meson8b_boot_rom,
	&meson8b_ahb_data_bus,
	&meson8b_ahb_ctrl_bus,
	&meson8b_hdmi_intr_sync,
	&meson8b_hdmi_pclk,
	&meson8b_usb1_ddr_bridge,
	&meson8b_usb0_ddr_bridge,
	&meson8b_mmc_pclk,
	&meson8b_dvin,
	&meson8b_uart2,
	&meson8b_sana,
	&meson8b_vpu_intr,
	&meson8b_sec_ahb_ahb3_bridge,
	&meson8b_clk81_a9,
	&meson8b_vclk2_venci0,
	&meson8b_vclk2_venci1,
	&meson8b_vclk2_vencp0,
	&meson8b_vclk2_vencp1,
	&meson8b_gclk_venci_int,
	&meson8b_gclk_vencp_int,
	&meson8b_dac_clk,
	&meson8b_aoclk_gate,
	&meson8b_iec958_gate,
	&meson8b_enc480p,
	&meson8b_rng1,
	&meson8b_gclk_vencl_int,
	&meson8b_vclk2_venclmcc,
	&meson8b_vclk2_vencl,
	&meson8b_vclk2_other,
	&meson8b_edp,
	&meson8b_ao_media_cpu,
	&meson8b_ao_ahb_sram,
	&meson8b_ao_ahb_bus,
	&meson8b_ao_iface,
	&meson8b_mpeg_clk_div,
	&meson8b_mpeg_clk_sel,
	&meson8b_mpll0,
	&meson8b_mpll1,
	&meson8b_mpll2,
	//&meson8b_mpll0_div,
	//&meson8b_mpll1_div,
	//&meson8b_mpll2_div,
	&meson8b_fixed_pll,
	&meson8b_sys_pll,
	&meson8b_cpu_in_sel,
	&meson8b_cpu_scale_div,
	&meson8b_cpu_scale_out_sel,
	&meson8b_cpu_clk,
	&meson8b_mpll_prediv,
	&meson8b_fclk_div2,
	&meson8b_fclk_div3,
	&meson8b_fclk_div4,
	&meson8b_fclk_div5,
	&meson8b_fclk_div7,
	&meson8b_nand_clk_sel,
	&meson8b_nand_clk_div,
	&meson8b_nand_clk_gate,
	&meson8b_fixed_pll_dco,
	//&meson8b_hdmi_pll_dco,
	//&meson8b_sys_pll_dco,
	&meson8b_apb_clk_sel,
	&meson8b_apb_clk_gate,
	&meson8b_periph_clk_sel,
	&meson8b_periph_clk_gate,
	&meson8b_axi_clk_sel,
	&meson8b_axi_clk_gate,
	&meson8b_l2_dram_clk_sel,
	&meson8b_l2_dram_clk_gate,
	&meson8b_hdmi_pll_lvds_out,
	&meson8b_hdmi_pll_hdmi_out,
	&meson8b_vid_pll_in_sel,
	&meson8b_vid_pll_in_en,
	&meson8b_vid_pll_pre_div,
	&meson8b_vid_pll_post_div,
	&meson8b_vid_pll,
	&meson8b_vid_pll_final_div,
	&meson8b_vclk_in_sel,
	&meson8b_vclk_in_en,
	&meson8b_vclk_en,
	&meson8b_vclk_div1_gate,
	&meson8b_vclk_div2_div_gate,
	&meson8b_vclk_div4_div_gate,
	&meson8b_vclk_div6_div_gate,
	&meson8b_vclk_div12_div_gate,
	&meson8b_vclk2_in_sel,
	&meson8b_vclk2_clk_in_en,
	&meson8b_vclk2_clk_en,
	&meson8b_vclk2_div1_gate,
	&meson8b_vclk2_div2_div_gate,
	&meson8b_vclk2_div4_div_gate,
	&meson8b_vclk2_div6_div_gate,
	&meson8b_vclk2_div12_div_gate,
	&meson8b_cts_enct_sel,
	&meson8b_cts_enct,
	&meson8b_cts_encp_sel,
	&meson8b_cts_encp,
	&meson8b_cts_enci_sel,
	&meson8b_cts_enci,
	&meson8b_hdmi_tx_pixel_sel,
	&meson8b_hdmi_tx_pixel,
	&meson8b_cts_encl_sel,
	&meson8b_cts_encl,
	&meson8b_cts_vdac0_sel,
	&meson8b_cts_vdac0,
	&meson8b_hdmi_sys_sel,
	&meson8b_hdmi_sys_div,
	&meson8b_hdmi_sys,
	&meson8b_mali_0_sel,
	&meson8b_mali_0_div,
	&meson8b_mali_0,
	&meson8b_mali_1_sel,
	&meson8b_mali_1_div,
	&meson8b_mali_1,
	&meson8b_mali,
	//&meson8m2_gp_pll_dco,
	&meson8m2_gp_pll,
	&meson8b_vpu_0_sel,
	&meson8m2_vpu_0_sel,
	&meson8b_vpu_0_div,
	&meson8b_vpu_0,
	&meson8b_vpu_1_sel,
	&meson8m2_vpu_1_sel,
	&meson8b_vpu_1_div,
	&meson8b_vpu_1,
	&meson8b_vpu,
	&meson8b_vdec_1_sel,
	&meson8b_vdec_1_1_div,
	&meson8b_vdec_1_1,
	&meson8b_vdec_1_2_div,
	&meson8b_vdec_1_2,
	&meson8b_vdec_1,
	&meson8b_vdec_hcodec_sel,
	&meson8b_vdec_hcodec_div,
	&meson8b_vdec_hcodec,
	&meson8b_vdec_2_sel,
	&meson8b_vdec_2_div,
	&meson8b_vdec_2,
	&meson8b_vdec_hevc_sel,
	&meson8b_vdec_hevc_div,
	&meson8b_vdec_hevc_en,
	&meson8b_vdec_hevc,
	&meson8b_cts_amclk,
	&meson8b_cts_amclk_sel,
	&meson8b_cts_amclk_div,
	&meson8b_cts_mclk_i958_sel,
	&meson8b_cts_mclk_i958_div,
	&meson8b_cts_mclk_i958,
	&meson8b_cts_i958,
	&meson8b_vid_pll_lvds_en,
};
#endif

#if 0
static const struct meson8b_clk_reset_line {
	u32 reg;
	u8 bit_idx;
	bool active_low;
} meson8b_clk_reset_bits[] = {
	[CLKC_RESET_L2_CACHE_SOFT_RESET] = {
		.reg = HHI_SYS_CPU_CLK_CNTL0,
		.bit_idx = 30,
		.active_low = false,
	},
	[CLKC_RESET_AXI_64_TO_128_BRIDGE_A5_SOFT_RESET] = {
		.reg = HHI_SYS_CPU_CLK_CNTL0,
		.bit_idx = 29,
		.active_low = false,
	},
	[CLKC_RESET_SCU_SOFT_RESET] = {
		.reg = HHI_SYS_CPU_CLK_CNTL0,
		.bit_idx = 28,
		.active_low = false,
	},
	[CLKC_RESET_CPU3_SOFT_RESET] = {
		.reg = HHI_SYS_CPU_CLK_CNTL0,
		.bit_idx = 27,
		.active_low = false,
	},
	[CLKC_RESET_CPU2_SOFT_RESET] = {
		.reg = HHI_SYS_CPU_CLK_CNTL0,
		.bit_idx = 26,
		.active_low = false,
	},
	[CLKC_RESET_CPU1_SOFT_RESET] = {
		.reg = HHI_SYS_CPU_CLK_CNTL0,
		.bit_idx = 25,
		.active_low = false,
	},
	[CLKC_RESET_CPU0_SOFT_RESET] = {
		.reg = HHI_SYS_CPU_CLK_CNTL0,
		.bit_idx = 24,
		.active_low = false,
	},
	[CLKC_RESET_A5_GLOBAL_RESET] = {
		.reg = HHI_SYS_CPU_CLK_CNTL0,
		.bit_idx = 18,
		.active_low = false,
	},
	[CLKC_RESET_A5_AXI_SOFT_RESET] = {
		.reg = HHI_SYS_CPU_CLK_CNTL0,
		.bit_idx = 17,
		.active_low = false,
	},
	[CLKC_RESET_A5_ABP_SOFT_RESET] = {
		.reg = HHI_SYS_CPU_CLK_CNTL0,
		.bit_idx = 16,
		.active_low = false,
	},
	[CLKC_RESET_AXI_64_TO_128_BRIDGE_MMC_SOFT_RESET] = {
		.reg = HHI_SYS_CPU_CLK_CNTL1,
		.bit_idx = 30,
		.active_low = false,
	},
	[CLKC_RESET_VID_CLK_CNTL_SOFT_RESET] = {
		.reg = HHI_VID_CLK_CNTL,
		.bit_idx = 15,
		.active_low = false,
	},
	[CLKC_RESET_VID_DIVIDER_CNTL_SOFT_RESET_POST] = {
		.reg = HHI_VID_DIVIDER_CNTL,
		.bit_idx = 7,
		.active_low = false,
	},
	[CLKC_RESET_VID_DIVIDER_CNTL_SOFT_RESET_PRE] = {
		.reg = HHI_VID_DIVIDER_CNTL,
		.bit_idx = 3,
		.active_low = false,
	},
	[CLKC_RESET_VID_DIVIDER_CNTL_RESET_N_POST] = {
		.reg = HHI_VID_DIVIDER_CNTL,
		.bit_idx = 1,
		.active_low = true,
	},
	[CLKC_RESET_VID_DIVIDER_CNTL_RESET_N_PRE] = {
		.reg = HHI_VID_DIVIDER_CNTL,
		.bit_idx = 0,
		.active_low = true,
	},
};

static int meson8b_clk_reset_update(struct reset_controller_dev *rcdev,
				    unsigned long id, bool assert)
{
	struct meson8b_clk_reset *meson8b_clk_reset =
		container_of(rcdev, struct meson8b_clk_reset, reset);
	const struct meson8b_clk_reset_line *reset;
	unsigned int value = 0;

	if (id >= ARRAY_SIZE(meson8b_clk_reset_bits))
		return -EINVAL;

	reset = &meson8b_clk_reset_bits[id];

	if (assert != reset->active_low)
		value = BIT(reset->bit_idx);

	regmap_update_bits(meson8b_clk_reset->regmap, reset->reg,
			   BIT(reset->bit_idx), value);

	return 0;
}

static int meson8b_clk_reset_assert(struct reset_controller_dev *rcdev,
				     unsigned long id)
{
	return meson8b_clk_reset_update(rcdev, id, true);
}

static int meson8b_clk_reset_deassert(struct reset_controller_dev *rcdev,
				       unsigned long id)
{
	return meson8b_clk_reset_update(rcdev, id, false);
}

static const struct reset_control_ops meson8b_clk_reset_ops = {
	.assert = meson8b_clk_reset_assert,
	.deassert = meson8b_clk_reset_deassert,
};
#endif

struct meson_clk_hw_data {
	struct clk_common_data **hws;
	unsigned int num;
};

#if 0
static struct meson_clk_hw_data meson8_clks = {
	.hws = meson8_hw_clks,
	.num = ARRAY_SIZE(meson8_hw_clks),
};
#endif

static struct meson_clk_hw_data meson8b_clks_data = {
	.hws = meson8b_clks,
	.num = ARRAY_SIZE(meson8b_clks),
};

#if 0
static struct meson_clk_hw_data meson8m2_clks = {
	.hws = meson8m2_hw_clks,
	.num = ARRAY_SIZE(meson8m2_hw_clks),
};
#endif

static void meson8b_clkc_init_common(struct udevice *dev,
				     struct meson_clk_hw_data *hw_clks)
{
	void __iomem *base;
	struct clk *clk;
	int i;

	/* use parent's base address */
	base = dev_read_addr_ptr(dev->parent);

	/* probe xtal first */
	devm_clk_get(dev, "xtal");

	/*
	 * register all clks and start with the first used ID (which is
	 * CLKID_PLL_FIXED)
	 */
	for (i = CLKID_PLL_FIXED; i < hw_clks->num; i++) {
		/* array might be sparse */
		if (!hw_clks->hws[i])
			continue;

		clk = meson_clk_register(dev, base, hw_clks->hws[i]);
		if (!clk)
			return;

		clk_dm(i, clk);
	}
}

static int meson8b_clkc_init(struct udevice *dev)
{
	meson8b_clkc_init_common(dev, &meson8b_clks_data);

	return 0;
}

static const struct udevice_id meson8b_clk_ids[] = {
	{ .compatible = "amlogic,meson8b-clkc" },
	{ }
};

U_BOOT_DRIVER(meson8b_clkc) = {
	.name		= "meson8b_clkc",
	.id		= UCLASS_CLK,
	.of_match	= meson8b_clk_ids,
	.ops		= &ccf_clk_ops,
	.probe		= meson8b_clkc_init,
	.flags		= DM_FLAG_PRE_RELOC,
};
