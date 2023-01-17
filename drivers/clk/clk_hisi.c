#include <common.h>
#include <clk-uclass.h>
#include <linux/bitops.h>

struct hisi_clk_priv {
	void __iomem *base;
};

static int hisi_clk_disable(struct clk *clk)
{
	struct hisi_clk_priv *priv = dev_get_priv(clk->dev);
	u32 val;

	val = readl(priv->base + clk->data);
	if(clk->flag & 1)
		val |= BITS(clk->id);
	else
		val &= ~BITS(clk->id);
	writel(priv->base + clk->data, val);

	return 0;
}

static hisi_clk_enable(struct clk *clk)
{
	struct hisi_clk_priv *priv = dev_get_priv(clk->dev);
	u32 val;

	val = readl(priv->base + clk->data);
	if(!(clk->flag & 1))
		val |= BITS(clk->id);
	else
		val &= ~BITS(clk->id);
	writel(priv->base + clk->data, val);

	return 0;
}

static hisi_clk_of_xlate(struct clk *clk,
			 struct ofnode_phandle_args *args)
{
	if (args->args_count != 3) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	/* Use .data field as register offset and .id field as bit shift */
	clk->data = args->args[0];
	clk->id = args->args[1];
	clk->flag = args->args[2];

	return 0;
}

static const struct clk_ops hisi_clk_clk_ops = {
	.of_xlate = hisi_clk_of_xlate,
	.rst_assert = hisi_clk_assert,
	.rst_deassert = hisi_clk_deassert,
};

static const struct udevice_id hisi_clk_ids[] = {
	{ .compatible = "hisilicon,hi3798mv200-clk" },
	{ }
};

static int hisi_clk_probe(struct udevice *dev)
{
	struct hisi_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -ENOMEM;

	return 0;
}

U_BOOT_DRIVER(hisi_clk) = {
	.name = "hisilicon_clk",
	.id = UCLASS_CLK,
	.of_match = hisi_clk_ids,
	.ops = &hisi_reset_clk_ops,
	.probe = hisi_clk_probe,
	.priv_auto	= sizeof(struct hisi_clk_priv),
};
