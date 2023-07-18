// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Hisilicon clock driver
 * Adapted from linux kernel
 */

#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/read.h>
#include <linux/clk-provider.h>
#include "clk.h"

static spinlock_t lock;

int hisi_clk_init(struct udevice *dev, struct hisi_clock_data *data)
{
	data->base = dev_remap_addr(dev);
	if (!data->base)
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

/**
 * hisi_clk_bind_reset - helper to bind reset driver
 *
 * Bind our device node to hisilicon_reset driver.
 * Can be used as the .bind member in crg driver.
 */
int hisi_clk_bind_reset(struct udevice *pdev)
{
	ofnode node = dev_ofnode(pdev);
	char *rst_dev_name;
	const char suffix[] = ".reset";
	int ret;

	rst_dev_name = malloc(strlen(pdev->name) + sizeof(suffix));
	if (!rst_dev_name) {
		dev_err(pdev, "out of memory\n");
		return log_msg_ret("clk", -ENOMEM);
	}

	strcpy(rst_dev_name, pdev->name);
	strcat(rst_dev_name, suffix);

	/* bind hisilicon_reset driver to our device tree node */
	ret = device_bind_driver_to_node(pdev, "hisilicon_reset", rst_dev_name, node, NULL);
	if (ret) {
		free(rst_dev_name);
		dev_err(pdev, "failed to bind to reset driver %d\n", ret);
		return log_msg_ret("clk", ret);
	}

	return 0;
}
