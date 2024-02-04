/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <dm/pinctrl.h>
#include <linux/bitops.h>

/* per-register bit definition */
#define HISTB_PIN_SCHMITT	BIT(14)
#define HISTB_PIN_PULLDOWN	BIT(13)
#define HISTB_PIN_PULLUP	BIT(12)
#define HISTB_PIN_SLEWRATE	BIT(8)
#define HISTB_PIN_DRV_MASK	GENMASK(7, 4)
#define HISTB_PIN_FUNC_MASK	GENMASK(2, 0)

/**
 * @histb_pin_desc flags
 *
 * @HISTB_PIN_FLAG_NOPU: This pin does not support "bias-pullup"
 * @HISTB_PIN_FLAG_NOPD: This pin does not support "bias-pulldown"
 * @HISTB_PIN_FLAG_NOSR: This pin does not support "slew-rate"
 * @HISTB_PIN_FLAG_HAS_SCHMITT: This pin supports setting schmitt
 */
#define HISTB_PIN_FLAG_NOPU	BIT(0)
#define HISTB_PIN_FLAG_NOPD	BIT(1)
#define HISTB_PIN_FLAG_NOSR	BIT(2)
#define HISTB_PIN_FLAG_HAS_SCHMITT	BIT(3)

/**
 * histb_pin_mux_desc - a descriptor for one function of a pin.
 *
 * @name: the name of the function, in capital
 * @bits: the bits pattern for this function
 */
struct histb_pin_mux_desc {
	const char *name;
	u32 bits;
};

/**
 * histb_pin_desc - a descriptor for a pin
 *
 * @number: ID for this pin, also used as offset into device address space
 * @name: pin name, e.g. "F17", refer to the datasheet for detail
 * @drv_tbl: (Optional) drive strength table, end with 0. The index is used as bits pattern. fill
 * NULL if setting drive strength is not supported.
 * @func_tbl: pinmux function table, end with { }.
 * @flags: pin flags (use HISTB_PIN_FLAG_* constants)
 */
struct histb_pin_desc {
	unsigned int number;
	const char *name;
	const u8 *drv_tbl;
	const struct histb_pin_mux_desc *func_tbl;
	int flags;
};

/**
 * HISTB_PIN() - a helper for initializing struct histb_pin_desc
 */
#define HISTB_PIN(_index, _name, _drv_tbl, _func_tbl, _flag) { \
	.number = _index, \
	.name = _name, \
	.drv_tbl = _drv_tbl, \
	.func_tbl = _func_tbl, \
	.flags = _flag, \
}

/**
 * histb_pin_func_setup_entry - an entry for setting up a function for a pin
 *
 * @pin_selector: pin ID
 * @mask: function bit pattern
 */
struct histb_pin_func_setup_entry {
	unsigned int pin_selector;
	u8 mask;
};

/**
 * histb_pin_function_desc - a descriptor for a function
 *
 * @func_selector: function selector
 * @name: function name
 * @pins_num: the number of pins engaged in this function
 * @cfg_tbl: config table
 */
struct histb_pin_function_desc {
	unsigned int func_selector;
	const char *name;
	unsigned int pins_num;
	const struct histb_pin_func_setup_entry *cfg_tbl;
};

/**
 * HISTB_PIN_FUNC - a helper macro to setup a function
 */
#define HISTB_PIN_FUNC(_id, _name, _cfg) { \
	.func_selector = _id, \
	.name = _name, \
	.pins_num = ARRAY_SIZE(_cfg), \
	.cfg_tbl = _cfg, \
}

/**
 * histb_pinctrl_priv - private data for the driver
 *
 * @base: device base address
 * @pins: descriptor array for all pins
 * @funcs: descriptor array for all functions
 * @pin_nums: array size of .pins
 * @func_nums: array size of .funcs
 */
struct histb_pinctrl_priv {
	void __iomem *base;
	const struct histb_pin_desc *pins;
	const struct histb_pin_function_desc *funcs;
	unsigned int pin_nums;
	unsigned int func_nums;
};

/**
 * histb_pinctrl_of_to_plat - convert device tree node to private data
 *
 * use this function as .of_to_plat field in the driver
 */
int histb_pinctrl_of_to_plat(struct udevice *dev);

/**
 * histb_pinctrl_ops - HiSTB pinctrl ops
 *
 * use this struct as .ops field in the driver
 */
extern const struct pinctrl_ops histb_pinctrl_ops;
