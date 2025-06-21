// SPDX-License-Identifier: GPL-2.0+
/*
 * Amlogic Meson6/Meson8/Meson8b/Meson8m2 SDHC MMC host controller driver.
 * Ported from linux
 *
 * Copyright (C) 2020 Martin Blumenstingl <martin.blumenstingl@googlemail.com>
 * Copyright (C) 2025 Yang Xiwen <forbidden405@outlook.com>
 */

#include <clk.h>
#include <mmc.h>
#include <pwrseq.h>
#include <regmap.h>
#include <dm/device_compat.h>
#include <dm/device.h>
#include <dm/of.h>
#include <linux/dma-mapping.h>
#include <linux/iopoll.h>
#include <linux/types.h>

#include "meson-mx-sdhc.h"

#define MESON_SDHC_NUM_BULK_CLKS				4
#define MESON_SDHC_MAX_BLK_SIZE					512
#define MESON_SDHC_NUM_TUNING_TRIES				10

#define MESON_SDHC_WAIT_CMD_READY_SLEEP_US			1
#define MESON_SDHC_WAIT_CMD_READY_TIMEOUT_US			100000
#define MESON_SDHC_WAIT_BEFORE_SEND_SLEEP_US			1
#define MESON_SDHC_WAIT_BEFORE_SEND_TIMEOUT_US			200

#define MESON_SDHC_POLL_IRQ_INTERVAL_US				100
#define MESON_SDHC_WAIT_IRQ_MS					10000

#define usleep_range(a, b) udelay((b))

struct meson_mx_sdhc_data {
	void		(*init_hw)(struct udevice *);
	void		(*set_pdma)(struct udevice *, struct mmc_data *);
	void		(*wait_before_send)(struct udevice *, struct mmc_data *);
	bool		hardware_flush_all_cmds;
};

struct meson_mx_sdhc_host_plat {
	struct mmc_config		cfg;
	struct mmc			mmc;

	struct regmap			*regmap;

	struct clk			pclk;
	struct clk			sd_clk;
	ulong				rate;

	const struct meson_mx_sdhc_data	*platform;
};

static void meson_mx_sdhc_reset(struct meson_mx_sdhc_host_plat *host)
{
	regmap_write(host->regmap, MESON_SDHC_SRST, MESON_SDHC_SRST_MAIN_CTRL |
		     MESON_SDHC_SRST_RXFIFO | MESON_SDHC_SRST_TXFIFO |
		     MESON_SDHC_SRST_DPHY_RX | MESON_SDHC_SRST_DPHY_TX |
		     MESON_SDHC_SRST_DMA_IF);
	usleep_range(10, 100);

	regmap_write(host->regmap, MESON_SDHC_SRST, 0);
	usleep_range(10, 100);
}

static void meson_mx_sdhc_clear_fifo(struct udevice *dev)
{
	struct meson_mx_sdhc_host_plat *host = dev_get_plat(dev);
	u32 stat;

	regmap_read(host->regmap, MESON_SDHC_STAT, &stat);
	if (!FIELD_GET(MESON_SDHC_STAT_RXFIFO_CNT, stat) &&
	    !FIELD_GET(MESON_SDHC_STAT_TXFIFO_CNT, stat))
		return;

	regmap_write(host->regmap, MESON_SDHC_SRST, MESON_SDHC_SRST_RXFIFO |
		     MESON_SDHC_SRST_TXFIFO | MESON_SDHC_SRST_MAIN_CTRL);
	udelay(5);

	regmap_read(host->regmap, MESON_SDHC_STAT, &stat);
	if (FIELD_GET(MESON_SDHC_STAT_RXFIFO_CNT, stat) ||
	    FIELD_GET(MESON_SDHC_STAT_TXFIFO_CNT, stat))
		dev_warn(dev,
			 "Failed to clear FIFOs, RX: %lu, TX: %lu\n",
			 FIELD_GET(MESON_SDHC_STAT_RXFIFO_CNT, stat),
			 FIELD_GET(MESON_SDHC_STAT_TXFIFO_CNT, stat));
}

static void meson_mx_sdhc_wait_cmd_ready(struct udevice *dev, struct mmc_cmd *cmd)
{
	struct meson_mx_sdhc_host_plat *host = dev_get_plat(dev);
	u32 stat, esta;
	int ret;

	ret = regmap_read_poll_timeout(host->regmap, MESON_SDHC_STAT, stat,
				       !(stat & MESON_SDHC_STAT_CMD_BUSY),
				       MESON_SDHC_WAIT_CMD_READY_SLEEP_US,
				       MESON_SDHC_WAIT_CMD_READY_TIMEOUT_US);
	if (ret) {
		dev_warn(dev,
			 "Failed to poll for CMD_BUSY while processing CMD%d\n",
			 cmd->cmdidx);
		meson_mx_sdhc_reset(host);
	}

	ret = regmap_read_poll_timeout(host->regmap, MESON_SDHC_ESTA, esta,
				       !(esta & MESON_SDHC_ESTA_11_13),
				       MESON_SDHC_WAIT_CMD_READY_SLEEP_US,
				       MESON_SDHC_WAIT_CMD_READY_TIMEOUT_US);
	if (ret) {
		dev_warn(dev,
			 "Failed to poll for ESTA[13:11] while processing CMD%d\n",
			 cmd->cmdidx);
		meson_mx_sdhc_reset(host);
	}
}

static u32 meson_mx_sdhc_read_response(struct udevice *dev, u8 idx)
{
	struct meson_mx_sdhc_host_plat *host = dev_get_plat(dev);
	u32 val;

	regmap_update_bits(host->regmap, MESON_SDHC_PDMA,
			   MESON_SDHC_PDMA_DMA_MODE, 0);

	regmap_update_bits(host->regmap, MESON_SDHC_PDMA,
			   MESON_SDHC_PDMA_PIO_RDRESP,
			   FIELD_PREP(MESON_SDHC_PDMA_PIO_RDRESP, idx));

	regmap_read(host->regmap, MESON_SDHC_ARGU, &val);

	return val;
}

static int meson_mx_sdhc_handle_irq(struct udevice *dev, struct mmc_cmd *cmd, struct mmc_data *data)
{
	struct meson_mx_sdhc_host_plat *host = dev_get_plat(dev);
	u32 ictl, ista;
	int error = 0;

	regmap_read(host->regmap, MESON_SDHC_ICTL, &ictl);
	regmap_read(host->regmap, MESON_SDHC_ISTA, &ista);

	if (ista & MESON_SDHC_ISTA_RXFIFO_FULL ||
	    ista & MESON_SDHC_ISTA_TXFIFO_EMPTY)
		error = -EIO;
	else if (ista & MESON_SDHC_ISTA_RESP_ERR_CRC)
		error = -EILSEQ;
	else if (ista & MESON_SDHC_ISTA_RESP_TIMEOUT)
		error = -ETIMEDOUT;

	if (data) {
		if (ista & MESON_SDHC_ISTA_DATA_ERR_CRC)
			error = -EILSEQ;
		else if (ista & MESON_SDHC_ISTA_DATA_TIMEOUT)
			error = -ETIMEDOUT;
	}

	if (error)
		dev_dbg(dev, "CMD%d error, ISTA: 0x%08x\n",
			cmd->cmdidx, ista);

	return error;
}

static void meson_mx_sdhc_read_resp(struct udevice *dev, struct mmc_cmd *cmd, struct mmc_data *data, int error)
{
	struct meson_mx_sdhc_host_plat *host = dev_get_plat(dev);
	u32 val;

	if (data) {
		if (!host->platform->hardware_flush_all_cmds &&
		    data->flags & MMC_DATA_READ) {
			meson_mx_sdhc_wait_cmd_ready(dev, cmd);

			/*
			 * If MESON_SDHC_PDMA_RXFIFO_MANUAL_FLUSH was
			 * previously 0x1 then it has to be set to 0x3. If it
			 * was 0x0 before then it has to be set to 0x2. Without
			 * this reading SD cards sometimes transfers garbage,
			 * which results in cards not being detected due to:
			 *   unrecognised SCR structure version <random number>
			 */
			val = FIELD_PREP(MESON_SDHC_PDMA_RXFIFO_MANUAL_FLUSH,
					 2);
			regmap_update_bits(host->regmap, MESON_SDHC_PDMA, val,
					   val);
		}
	}

	meson_mx_sdhc_wait_cmd_ready(dev, cmd);

	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[0] = meson_mx_sdhc_read_response(dev, 4);
		cmd->response[1] = meson_mx_sdhc_read_response(dev, 3);
		cmd->response[2] = meson_mx_sdhc_read_response(dev, 2);
		cmd->response[3] = meson_mx_sdhc_read_response(dev, 1);
	} else {
		cmd->response[0] = meson_mx_sdhc_read_response(dev, 0);
	}

	if (error == -ETIMEDOUT || error == -EIO)
		meson_mx_sdhc_reset(host);
	else if (data)
		/*
		 * Clear the FIFOs after completing data transfers to prevent
		 * corrupting data on write access. It's not clear why this is
		 * needed (for reads and writes), but it mimics what the BSP
		 * kernel did.
		 */
		meson_mx_sdhc_clear_fifo(dev);

	/* disable interrupts and mask all pending ones */
	regmap_update_bits(host->regmap, MESON_SDHC_ICTL,
			   MESON_SDHC_ICTL_ALL_IRQS, 0);
	regmap_update_bits(host->regmap, MESON_SDHC_ISTA,
			   MESON_SDHC_ISTA_ALL_IRQS, MESON_SDHC_ISTA_ALL_IRQS);
}

static int meson_mx_sdhc_start_cmd(struct udevice *dev,
				    struct mmc_cmd *cmd,
				    struct mmc_data *data)
{
	struct meson_mx_sdhc_host_plat *host = dev_get_plat(dev);
	u32 ictl, send, ista;
	int pack_len, ret;
	ulong ts;

	ictl = MESON_SDHC_ICTL_DATA_TIMEOUT | MESON_SDHC_ICTL_DATA_ERR_CRC |
	       MESON_SDHC_ICTL_RXFIFO_FULL | MESON_SDHC_ICTL_TXFIFO_EMPTY |
	       MESON_SDHC_ICTL_RESP_TIMEOUT | MESON_SDHC_ICTL_RESP_ERR_CRC;

	send = FIELD_PREP(MESON_SDHC_SEND_CMD_INDEX, cmd->cmdidx);

	if (data) {
		send |= MESON_SDHC_SEND_CMD_HAS_DATA;
		send |= FIELD_PREP(MESON_SDHC_SEND_TOTAL_PACK,
				   data->blocks - 1);

		if (data->blocksize < MESON_SDHC_MAX_BLK_SIZE)
			pack_len = data->blocksize;
		else
			pack_len = 0;

		if (data->flags & MMC_DATA_WRITE)
			send |= MESON_SDHC_SEND_DATA_DIR;

		/*
		 * If command with no data, just wait response done
		 * interrupt(int[0]), and if command with data transfer, just
		 * wait dma done interrupt(int[11]), don't need care about
		 * dat0 busy or not.
		 */
		if (host->platform->hardware_flush_all_cmds ||
		    data->flags & MMC_DATA_WRITE)
			/* hardware flush: */
			ictl |= MESON_SDHC_ICTL_DMA_DONE;
		else
			/* software flush: */
			ictl |= MESON_SDHC_ICTL_DATA_XFER_OK;
	} else {
		pack_len = 0;

		ictl |= MESON_SDHC_ICTL_RESP_OK;
	}

	/* We should set this bit since CMD23 is not supported in U-Boot */
	regmap_update_bits(host->regmap, MESON_SDHC_MISC,
			   MESON_SDHC_MISC_MANUAL_STOP,
			   FIELD_PREP(MESON_SDHC_MISC_MANUAL_STOP, 0b1));

	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION) {
		send |= MESON_SDHC_SEND_DATA_STOP;
	}

	if (cmd->resp_type & MMC_RSP_PRESENT)
		send |= MESON_SDHC_SEND_CMD_HAS_RESP;

	if (cmd->resp_type & MMC_RSP_136) {
		send |= MESON_SDHC_SEND_RESP_LEN;
		send |= MESON_SDHC_SEND_RESP_NO_CRC;
	}

	if (!(cmd->resp_type & MMC_RSP_CRC))
		send |= MESON_SDHC_SEND_RESP_NO_CRC;

	if (cmd->resp_type & MMC_RSP_BUSY)
		send |= MESON_SDHC_SEND_R1B;

	/* enable the new IRQs and mask all pending ones */
	regmap_write(host->regmap, MESON_SDHC_ICTL, ictl);
	regmap_write(host->regmap, MESON_SDHC_ISTA, MESON_SDHC_ISTA_ALL_IRQS);

	regmap_write(host->regmap, MESON_SDHC_ARGU, cmd->cmdarg);

	regmap_update_bits(host->regmap, MESON_SDHC_CTRL,
			   MESON_SDHC_CTRL_PACK_LEN,
			   FIELD_PREP(MESON_SDHC_CTRL_PACK_LEN, pack_len));

	/* Flush cache before updating MESON_SDHC_ADDR */
	if (data && data->flags & MMC_DATA_WRITE) {
		ulong data_addr = (ulong) data->src;
		ulong data_size = data->blocks * data->blocksize;
		flush_dcache_range(data_addr, data_addr + data_size);
	}

	/* NOTE: DMA starts as soon as MESON_SDHC_ADDR is updated */
	if (data)
		regmap_write(host->regmap, MESON_SDHC_ADDR,
			     (ulong)data->dest);

	meson_mx_sdhc_wait_cmd_ready(dev, cmd);

	if (data)
		host->platform->set_pdma(dev, data);

	if (host->platform->wait_before_send)
		host->platform->wait_before_send(dev, data);

	regmap_write(host->regmap, MESON_SDHC_SEND, send);

	ts = get_timer_us_long(0);
	ret = regmap_read_poll_timeout(host->regmap, MESON_SDHC_ISTA, ista, ista & ictl,
				       MESON_SDHC_POLL_IRQ_INTERVAL_US, MESON_SDHC_WAIT_IRQ_MS);
	ts = get_timer_us_long(ts);
	debug("spent %lu us to poll IRQ\n", ts);

	if (ret)
		return ret;

	ret = meson_mx_sdhc_handle_irq(dev, cmd, data);
	if (ret)
		return ret;

	meson_mx_sdhc_read_resp(dev, cmd, data, ret);

	if (data && data->flags & MMC_DATA_READ) {
		ulong data_addr = (ulong) data->dest;
		ulong data_size = data->blocks * data->blocksize;
		invalidate_dcache_range(data_addr, data_addr + data_size);
	}

	return ret;
}

static void meson_mx_sdhc_enable_clks(struct udevice *dev)
{
	struct meson_mx_sdhc_host_plat *host = dev_get_plat(dev);
	u32 mask, val;

	/* Select FCLK_DIV3 as the sd_clk input */
	mask = MESON_SDHC_CLKC_CLK_IN_SEL;
	val = FIELD_PREP(MESON_SDHC_CLKC_CLK_IN_SEL, MESON_SDHC_CLK_FCLK_DIV3);
	regmap_update_bits(host->regmap, MESON_SDHC_CLKC, mask, val);

	/* Enable clocks */
	mask = val = MESON_SDHC_CLKC_TX_CLK_ON | MESON_SDHC_CLKC_RX_CLK_ON |
		     MESON_SDHC_CLKC_SD_CLK_ON | MESON_SDHC_CLKC_MOD_CLK_ON;
	regmap_update_bits(host->regmap, MESON_SDHC_CLKC, mask, val);
}

static void meson_mx_sdhc_disable_clks(struct udevice *dev)
{
	struct meson_mx_sdhc_host_plat *host = dev_get_plat(dev);
	u32 mask;

	/* Disable clocks */
	mask = MESON_SDHC_CLKC_TX_CLK_ON | MESON_SDHC_CLKC_RX_CLK_ON |
		MESON_SDHC_CLKC_SD_CLK_ON | MESON_SDHC_CLKC_MOD_CLK_ON;
	regmap_update_bits(host->regmap, MESON_SDHC_CLKC, mask, 0);
}

static int meson_mx_sdhc_set_clk(struct udevice *dev)
{
	struct meson_mx_sdhc_host_plat *host = dev_get_plat(dev);
	struct mmc *mmc = &host->mmc;
	u32 val, rx_clk_phase, clock;

	if (!mmc->clk_disable) {
		clock = clamp(mmc->clock, mmc->cfg->f_min, mmc->cfg->f_max);
		u32 divisor = (DIV_ROUND_UP(host->rate, mmc->clock) - 1);
		divisor = FIELD_PREP(MESON_SDHC_CLKC_CLK_DIV, divisor);

		meson_mx_sdhc_disable_clks(dev);

		debug("Setting clock to %u Hz\n", clock);
		regmap_update_bits(host->regmap, MESON_SDHC_CLKC, MESON_SDHC_CLKC_CLK_DIV, divisor);

		meson_mx_sdhc_enable_clks(dev);

		mmc->clock = host->rate / (divisor + 1);

		/*
		 * Phase 90 should work in most cases. For data transmission,
		 * meson_mx_sdhc_execute_tuning() will find a accurate value
		 */
		regmap_read(host->regmap, MESON_SDHC_CLKC, &val);
		rx_clk_phase = FIELD_GET(MESON_SDHC_CLKC_CLK_DIV, val) / 4;
		regmap_update_bits(host->regmap, MESON_SDHC_CLK2,
				   MESON_SDHC_CLK2_RX_CLK_PHASE,
				   FIELD_PREP(MESON_SDHC_CLK2_RX_CLK_PHASE,
					      rx_clk_phase));
	} else {
		meson_mx_sdhc_disable_clks(dev);
		mmc->clock = 0;
	}

	return 0;
}

static int meson_mx_sdhc_set_ios(struct udevice *dev)
{
	struct meson_mx_sdhc_host_plat *host = dev_get_plat(dev);
	struct mmc *mmc = &host->mmc;

	meson_mx_sdhc_set_clk(dev);

	switch (mmc->bus_width) {
	case 1:
		regmap_update_bits(host->regmap, MESON_SDHC_CTRL,
				   MESON_SDHC_CTRL_DAT_TYPE,
				   FIELD_PREP(MESON_SDHC_CTRL_DAT_TYPE, 0));
		break;

	case 4:
		regmap_update_bits(host->regmap, MESON_SDHC_CTRL,
				   MESON_SDHC_CTRL_DAT_TYPE,
				   FIELD_PREP(MESON_SDHC_CTRL_DAT_TYPE, 1));
		break;

	case 8:
		regmap_update_bits(host->regmap, MESON_SDHC_CTRL,
				   MESON_SDHC_CTRL_DAT_TYPE,
				   FIELD_PREP(MESON_SDHC_CTRL_DAT_TYPE, 2));
		break;

	default:
		dev_dbg(dev, "unsupported bus width: %d\n",
			mmc->bus_width);
		return -EINVAL;
	}

	return 0;
}

static const struct dm_mmc_ops meson_mx_sdhc_ops = {
	.send_cmd			= meson_mx_sdhc_start_cmd,
	.set_ios			= meson_mx_sdhc_set_ios,
};


static void meson_mx_sdhc_init_hw_meson8(struct udevice *dev)
{
	struct meson_mx_sdhc_host_plat *host = dev_get_plat(dev);

	regmap_write(host->regmap, MESON_SDHC_MISC,
		     FIELD_PREP(MESON_SDHC_MISC_TXSTART_THRES, 7) |
		     FIELD_PREP(MESON_SDHC_MISC_WCRC_ERR_PATT, 5) |
		     FIELD_PREP(MESON_SDHC_MISC_WCRC_OK_PATT, 2));

	regmap_write(host->regmap, MESON_SDHC_ENHC,
		     FIELD_PREP(MESON_SDHC_ENHC_RXFIFO_TH, 63) |
		     MESON_SDHC_ENHC_MESON6_DMA_WR_RESP |
		     FIELD_PREP(MESON_SDHC_ENHC_MESON6_RX_TIMEOUT, 255) |
		     FIELD_PREP(MESON_SDHC_ENHC_SDIO_IRQ_PERIOD, 12));
};

static void meson_mx_sdhc_set_pdma_meson8(struct udevice *dev, struct mmc_data *data)
{
	struct meson_mx_sdhc_host_plat *host = dev_get_plat(dev);

	if (data->flags & MMC_DATA_WRITE)
		regmap_update_bits(host->regmap, MESON_SDHC_PDMA,
				   MESON_SDHC_PDMA_DMA_MODE |
				   MESON_SDHC_PDMA_RD_BURST |
				   MESON_SDHC_PDMA_TXFIFO_FILL,
				   MESON_SDHC_PDMA_DMA_MODE |
				   FIELD_PREP(MESON_SDHC_PDMA_RD_BURST, 31) |
				   MESON_SDHC_PDMA_TXFIFO_FILL);
	else
		regmap_update_bits(host->regmap, MESON_SDHC_PDMA,
				   MESON_SDHC_PDMA_DMA_MODE |
				   MESON_SDHC_PDMA_RXFIFO_MANUAL_FLUSH,
				   MESON_SDHC_PDMA_DMA_MODE |
				   FIELD_PREP(MESON_SDHC_PDMA_RXFIFO_MANUAL_FLUSH,
					      0));

	if (data->flags & MMC_DATA_WRITE)
		regmap_update_bits(host->regmap, MESON_SDHC_PDMA,
				   MESON_SDHC_PDMA_RD_BURST,
				   FIELD_PREP(MESON_SDHC_PDMA_RD_BURST, 15));
}

static void meson_mx_sdhc_wait_before_send_meson8(struct udevice *dev, struct mmc_data *data)
{
	struct meson_mx_sdhc_host_plat *host = dev_get_plat(dev);
	u32 val;
	int ret;

	ret = regmap_read_poll_timeout(host->regmap, MESON_SDHC_ESTA, val,
				       val == 0,
				       MESON_SDHC_WAIT_BEFORE_SEND_SLEEP_US,
				       MESON_SDHC_WAIT_BEFORE_SEND_TIMEOUT_US);
	if (ret)
		dev_warn(dev,
			 "Failed to wait for ESTA to clear: 0x%08x\n", val);

	if (data && data->flags & MMC_DATA_WRITE) {
		ret = regmap_read_poll_timeout(host->regmap, MESON_SDHC_STAT,
					val, val & MESON_SDHC_STAT_TXFIFO_CNT,
					MESON_SDHC_WAIT_BEFORE_SEND_SLEEP_US,
					MESON_SDHC_WAIT_BEFORE_SEND_TIMEOUT_US);
		if (ret)
			dev_warn(dev,
				 "Failed to wait for TX FIFO to fill\n");
	}
}

static void meson_mx_sdhc_init_hw_meson8m2(struct udevice *dev)
{
	struct meson_mx_sdhc_host_plat *host = dev_get_plat(dev);

	regmap_write(host->regmap, MESON_SDHC_MISC,
		     FIELD_PREP(MESON_SDHC_MISC_TXSTART_THRES, 6) |
		     FIELD_PREP(MESON_SDHC_MISC_WCRC_ERR_PATT, 5) |
		     FIELD_PREP(MESON_SDHC_MISC_WCRC_OK_PATT, 2));

	regmap_write(host->regmap, MESON_SDHC_ENHC,
		     FIELD_PREP(MESON_SDHC_ENHC_RXFIFO_TH, 64) |
		     FIELD_PREP(MESON_SDHC_ENHC_MESON8M2_DEBUG, 1) |
		     MESON_SDHC_ENHC_MESON8M2_WRRSP_MODE |
		     FIELD_PREP(MESON_SDHC_ENHC_SDIO_IRQ_PERIOD, 12));
}

static void meson_mx_sdhc_set_pdma_meson8m2(struct udevice *dev, struct mmc_data *data)
{
	struct meson_mx_sdhc_host_plat *host = dev_get_plat(dev);

	regmap_update_bits(host->regmap, MESON_SDHC_PDMA,
			   MESON_SDHC_PDMA_DMA_MODE, MESON_SDHC_PDMA_DMA_MODE);
}

static void meson_mx_sdhc_init_hw(struct udevice *dev)
{
	struct meson_mx_sdhc_host_plat *host = dev_get_plat(dev);

	meson_mx_sdhc_reset(host);

	regmap_write(host->regmap, MESON_SDHC_CTRL,
		     FIELD_PREP(MESON_SDHC_CTRL_RX_PERIOD, 0xf) |
		     FIELD_PREP(MESON_SDHC_CTRL_RX_TIMEOUT, 0x7f) |
		     FIELD_PREP(MESON_SDHC_CTRL_RX_ENDIAN, 0x7) |
		     FIELD_PREP(MESON_SDHC_CTRL_TX_ENDIAN, 0x7));

	/*
	 * start with a valid divider and enable the memory (un-setting
	 * MESON_SDHC_CLKC_MEM_PWR_OFF).
	 */
	regmap_write(host->regmap, MESON_SDHC_CLKC, MESON_SDHC_CLKC_CLK_DIV);

	regmap_write(host->regmap, MESON_SDHC_CLK2,
		     FIELD_PREP(MESON_SDHC_CLK2_SD_CLK_PHASE, 1));

	regmap_write(host->regmap, MESON_SDHC_PDMA,
		     MESON_SDHC_PDMA_DMA_URGENT |
		     FIELD_PREP(MESON_SDHC_PDMA_WR_BURST, 7) |
		     FIELD_PREP(MESON_SDHC_PDMA_TXFIFO_TH, 49) |
		     FIELD_PREP(MESON_SDHC_PDMA_RD_BURST, 15) |
		     FIELD_PREP(MESON_SDHC_PDMA_RXFIFO_TH, 7));

	/* some initialization bits depend on the SoC: */
	host->platform->init_hw(dev);

	/* disable and mask all interrupts: */
	regmap_write(host->regmap, MESON_SDHC_ICTL, 0);
	regmap_write(host->regmap, MESON_SDHC_ISTA, MESON_SDHC_ISTA_ALL_IRQS);
}

static int meson_mx_sdhc_probe(struct udevice *dev)
{
	struct meson_mx_sdhc_host_plat *host;
	struct mmc_config *cfg;
	struct mmc *mmc;
	int ret;

	host = dev_get_plat(dev);
	cfg = &host->cfg;
	mmc = &host->mmc;

	host->platform = (void *)dev_get_driver_data(dev);
	if (!host->platform)
		return -EINVAL;

	ret = regmap_init_mem(dev_ofnode(dev), &host->regmap);
	if (ret)
		return ret;

	ret = clk_get_by_name(dev, "pclk", &host->pclk);
	if (ret) {
		dev_err(dev, "Failed to get pclk: %d\n", ret);
		return ret;
	}

	/* Simply choose FCLK_DIV3 (850MHz for meson8b) */
	ret = clk_get_by_name(dev, "clkin2", &host->sd_clk);
	if (ret) {
		dev_err(dev, "Failed to get clkin2: %d\n", ret);
		return ret;
	}

	/* accessing any register requires the module clock to be enabled: */
	ret = clk_enable(&host->pclk);
	if (ret) {
		dev_err(dev, "Failed to enable pclk: %d\n", ret);
		return ret;
	}

	ret = clk_enable(&host->sd_clk);
	if (ret) {
		dev_err(dev, "Failed to enable sd_clk: %d\n", ret);
		return ret;
	}

	host->rate = clk_get_rate(&host->sd_clk);
	if (IS_ERR_VALUE(host->rate)) {
		dev_err(dev, "Failed to get sd_clk rate: %d\n", (int)host->rate);
		return (int)host->rate;
	}

	debug("sdclk rate is %lu Hz\n", host->rate);

	meson_mx_sdhc_init_hw(dev);

	meson_mx_sdhc_enable_clks(dev);

#if CONFIG_IS_ENABLED(MMC_PWRSEQ)
	/* Reset the card */
	ret = mmc_pwrseq_get_power(dev, cfg);
	if (!ret) {
		ret = pwrseq_set_power(cfg->pwr_dev, true);
		if (ret)
			return ret;
	}
#endif

	return mmc_init(mmc);
}

static int meson_mx_sdhc_of_to_plat(struct udevice *dev)
{
	struct meson_mx_sdhc_host_plat *plat = dev_get_plat(dev);
	struct mmc_config *cfg = &plat->cfg;

	cfg->name = dev->name;
	cfg->b_max = FIELD_GET(MESON_SDHC_SEND_TOTAL_PACK, ~0); 
	cfg->voltages = MMC_VDD_33_34 | MMC_VDD_32_33 |
			MMC_VDD_31_32 | MMC_VDD_165_195;
	/* eMMC spec says the maximum clock rate is 400 kHz in identification mode */
	cfg->f_min = 400000; /* 400 kHz */

	return mmc_of_parse(dev, cfg);
}

static int meson_mx_sdhc_bind(struct udevice *dev)
{
	struct meson_mx_sdhc_host_plat *plat = dev_get_plat(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct meson_mx_sdhc_data meson_mx_sdhc_data_meson8 = {
	.init_hw			= meson_mx_sdhc_init_hw_meson8,
	.set_pdma			= meson_mx_sdhc_set_pdma_meson8,
	.wait_before_send		= meson_mx_sdhc_wait_before_send_meson8,
	.hardware_flush_all_cmds	= false,
};

static const struct meson_mx_sdhc_data meson_mx_sdhc_data_meson8m2 = {
	.init_hw			= meson_mx_sdhc_init_hw_meson8m2,
	.set_pdma			= meson_mx_sdhc_set_pdma_meson8m2,
	.hardware_flush_all_cmds	= true,
};

static const struct udevice_id meson_mx_sdhc_of_match[] = {
	{
		.compatible = "amlogic,meson8-sdhc",
		.data = (ulong)&meson_mx_sdhc_data_meson8
	},
	{
		.compatible = "amlogic,meson8b-sdhc",
		.data = (ulong)&meson_mx_sdhc_data_meson8
	},
	{
		.compatible = "amlogic,meson8m2-sdhc",
		.data = (ulong)&meson_mx_sdhc_data_meson8m2
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(meson_mx_sdhc_driver) = {
	.name		= "meson-mx-sdhc",
	.id		= UCLASS_MMC,
	.of_match	= meson_mx_sdhc_of_match,
	.ops		= &meson_mx_sdhc_ops,
	.bind		= meson_mx_sdhc_bind,
	.of_to_plat	= meson_mx_sdhc_of_to_plat,
	.probe		= meson_mx_sdhc_probe,
	.plat_auto	= sizeof(struct meson_mx_sdhc_host_plat),
};

