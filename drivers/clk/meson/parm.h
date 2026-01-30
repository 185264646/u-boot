/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2015 Endless Mobile, Inc.
 * Author: Carlo Caione <carlo@endlessm.com>
 */

#ifndef __MESON_PARM_H
#define __MESON_PARM_H

#include <asm/io.h>
#include <linux/bitops.h>

#define PMASK(width)			GENMASK(width - 1, 0)
#define SETPMASK(width, shift)		GENMASK(shift + width - 1, shift)
#define CLRPMASK(width, shift)		(~SETPMASK(width, shift))

#define PARM_GET(width, shift, reg)					\
	(((reg) & SETPMASK(width, shift)) >> (shift))
#define PARM_SET(width, shift, reg, val)				\
	(((reg) & CLRPMASK(width, shift)) | ((val) << (shift)))

#define MESON_PARM_APPLICABLE(p)		(!!((p)->width))

struct parm {
	u16	reg_off;
	u8	shift;
	u8	width;
};

static inline unsigned int meson_parm_read(void *base, const struct parm *p)
{
	unsigned int val;

	val = readl(base + p->reg_off);
	return PARM_GET(p->width, p->shift, val);
}

static inline void meson_parm_write(void *base, const struct parm *p,
				    unsigned int val)
{
	clrsetbits_32(base + p->reg_off, SETPMASK(p->width, p->shift),
		      val << p->shift);
}

#endif /* __MESON_PARM_H */

