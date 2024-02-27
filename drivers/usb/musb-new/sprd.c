// SPDX-License-Identifier: GPL-2.0
/*
 * SpreadTrum MUSB "glue layer"
 *
 * Copyright © 2024 Yang Xiwen <forbidden405@outlook.com>
 *
 * Based on SUNXI glue driver
 *
 */
#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/usb/musb.h>
#include "linux-compat.h"
#include "musb_core.h"
#include "musb_uboot.h"

struct sprd_musb_config {
	struct musb_hdrc_platform_data *pdata;
};

struct sprd_glue {
	struct musb_host_data mdata;
	struct device dev;
};
#define to_sprd_glue(d)	container_of(d, struct sprd_glue, dev)

/******************************************************************************
 * MUSB Glue code
 ******************************************************************************/

int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
	struct sprd_glue *glue = dev_get_priv(dev);
	struct musb *gadget = glue->mdata.host;

	schedule();
	if (!gadget || !gadget->isr)
		return -EINVAL;

	return gadget->isr(0, gadget);
}

#if 0
static irqreturn_t sprd_musb_interrupt(int irq, void *__hci)
{
	struct musb		*musb = __hci;
	irqreturn_t		retval = IRQ_NONE;

	/* read and flush interrupts */
	musb->int_usb = musb_readb(musb->mregs, MUSB_INTRUSB);
	if (musb->int_usb)
		musb_writeb(musb->mregs, MUSB_INTRUSB, musb->int_usb);
	musb->int_tx = musb_readw(musb->mregs, MUSB_INTRTX);
	if (musb->int_tx)
		musb_writew(musb->mregs, MUSB_INTRTX, musb->int_tx);
	musb->int_rx = musb_readw(musb->mregs, MUSB_INTRRX);
	if (musb->int_rx)
		musb_writew(musb->mregs, MUSB_INTRRX, musb->int_rx);

	if (musb->int_usb || musb->int_tx || musb->int_rx)
		retval |= musb_interrupt(musb);

	return retval;
}
#endif

static int sprd_musb_enable(struct musb *musb)
{
	struct sprd_glue *glue = to_sprd_glue(musb->controller);
	int ret;

	pr_debug("%s():\n", __func__);

#if 0
	musb_ep_select(musb->mregs, 0);
	musb_writeb(musb->mregs, MUSB_FADDR, 0);
#endif

	return 0;
}

static void sprd_musb_disable(struct musb *musb)
{
	pr_debug("%s():\n", __func__);
}

static int sprd_musb_init(struct musb *musb)
{
	void __iomem *base = musb->mregs;

	musb_writeb(base, MUSB_POWER, MUSB_POWER_SOFTCONN | MUSB_POWER_HSENAB);
	musb_writeb(base, MUSB_INTRUSBE, 0xFF);

	pr_debug("%s():\n", __func__);

	return 0;
}

static int sprd_musb_exit(struct musb *musb)
{
	return 0;
}

static const struct musb_platform_ops sprd_musb_ops = {
	.init		= sprd_musb_init,
	.exit		= sprd_musb_exit,
	.enable		= sprd_musb_enable,
	.disable	= sprd_musb_disable,
};

static struct musb_hdrc_config musb_config = {
	.multipoint	= true,
	.dyn_fifo	= true,
	.num_eps	= 7,
	.ram_bits	= 11,
	.dma		= 0,
};

static struct musb_hdrc_platform_data sprd_musb_pdata = {
	.mode		= MUSB_PERIPHERAL,
	.config		= &musb_config,
	.power		= 250,
	.platform_ops	= &sprd_musb_ops,
};

static int musb_usb_probe(struct udevice *dev)
{
	struct sprd_glue *glue = dev_get_priv(dev);
	struct musb_host_data *host = &glue->mdata;
	struct sprd_musb_config *cfg;
	void *base = dev_read_addr_ptr(dev);

	if (!base)
		return -EINVAL;

	cfg = (struct sprd_musb_config *)dev_get_driver_data(dev);
	if (!cfg)
		return -EINVAL;

	host->host = musb_init_controller(cfg->pdata, &glue->dev, base);
	if (IS_ERR_OR_NULL(host->host))
		return -EIO;

	return usb_add_gadget_udc(&glue->dev, &host->host->g);
}

static int musb_usb_remove(struct udevice *dev)
{
	struct sprd_glue *glue = dev_get_priv(dev);
	struct musb_host_data *host = &glue->mdata;

	usb_del_gadget_udc(&host->host->g);
	musb_stop(host->host);
	free(host->host);
	host->host = NULL;

	return 0;
}

static const struct sprd_musb_config sharkl3_cfg = {
	.pdata = &sprd_musb_pdata,
};

static const struct udevice_id sprd_musb_ids[] = {
	{
		.compatible = "sprd,sharkl3-musb",
		.data = (ulong)&sharkl3_cfg,
	}, {
		/* terminator */
	}
};

U_BOOT_DRIVER(sprd_musb) = {
	.name		= "sprd-musb",
	.id		= UCLASS_USB_GADGET_GENERIC,
	.of_match	= sprd_musb_ids,
	.probe		= musb_usb_probe,
	.remove		= musb_usb_remove,
	.plat_auto	= sizeof(struct usb_plat),
	.priv_auto	= sizeof(struct sprd_glue),
};
