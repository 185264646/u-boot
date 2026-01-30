// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/types.h>
#include <linux/bitfield.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dm/read.h>
#include <timer.h>

#define MESON_ISA_TIMER_MUX		0x0
#define MESON_TIMERE_CLK_SEL_MASK	GENMASK(10,8)
#define MESON_TIMERE_CLK_SEL_SYS	0
#define MESON_TIMERE_CLK_SEL_1US	1
#define MESON_TIMERE_CLK_SEL_10US	2
#define MESON_TIMERE_CLK_SEL_100US	3
#define MESON_TIMERE_CLK_SEL_1MS	4
#define MESON_ISA_TIMERE		0x14

struct meson_timer_plat {
	void __iomem *base;
	u32 freq;
};

static u64 meson_timer_get_count(struct udevice *dev)
{
	struct meson_timer_plat *plat = dev_get_plat(dev);
	uint32_t cntr = readl(plat->base + MESON_ISA_TIMERE);

	return cntr;
}

static const struct timer_ops meson_timer_ops = {
	.get_count	= meson_timer_get_count,
};

static int meson_timer_of_to_plat(struct udevice *dev)
{
	struct meson_timer_plat *plat = dev_get_plat(dev);

	plat->base = dev_read_addr_ptr(dev);
	if (!plat->base)
		return -ENOENT;

	plat->freq = dev_read_u32_default(dev, "clock-frequency", 0);

	return 0;
}

static int meson_timer_probe(struct udevice *dev)
{
	struct meson_timer_plat *plat = dev_get_plat(dev);
	u32 val, field;

	if (plat->freq == 1000000)
		field = MESON_TIMERE_CLK_SEL_1US;
	else if (plat->freq == 100000)
		field = MESON_TIMERE_CLK_SEL_10US;
	else if (plat->freq == 10000)
		field = MESON_TIMERE_CLK_SEL_100US;
	else if (plat->freq == 1000)
		field = MESON_TIMERE_CLK_SEL_1MS;
	else
		/* TODO: allow SYS_CLK to be used here */
		return -EINVAL;

	val = readl(plat->base + MESON_ISA_TIMER_MUX);
	val &= ~MESON_TIMERE_CLK_SEL_MASK;
	val |= FIELD_PREP(MESON_TIMERE_CLK_SEL_MASK, field);

	writel(val, plat->base + MESON_ISA_TIMER_MUX);

	return 0;
}

static const struct udevice_id meson_timer_ids[] = {
	{ .compatible = "amlogic,meson6-timer" },
	{},
};

U_BOOT_DRIVER(meson_timer) = {
	.name		= "meson6_timer",
	.id		= UCLASS_TIMER,
	.of_match	= meson_timer_ids,
	.probe		= meson_timer_probe,
	.ops		= &meson_timer_ops,
	.plat_auto	= sizeof(struct meson_timer_plat),
	.of_to_plat	= meson_timer_of_to_plat,
};
