// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2023 (R) Yang Xiwen
 */

#include <ctype.h>
#include <command.h>
#include <config.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <env.h>
#include <usb.h>
#include <asm/io.h>
#include <linux/bitops.h>

#define AP_AHB_CTRL_REG_BASE	0x20E00000

#define AHB_EB_ADDR		0
#define AHB_EB_OTG_EB_MASK	GENMASK(4, 4)
#define AHB_EB_DMA_EB_MASK	GENMASK(5, 5)
#define AHB_EB_CE_EB_MASK	GENMASK(6, 6)

#define AHB_RST_ADDR		4

/* copied from a downstream u-boot */
static struct console_t {
	short curr_col, curr_row;
	short cols, rows;
	void *fbbase;
	u32 lcdsizex, lcdsizey, lcdrot;
	void (*fp_putc_xy)(struct console_t *pcons, ushort x, ushort y, char c);
	void (*fp_console_moverow)(struct console_t *pcons,
				   u32 rowdst, u32 rowsrc);
	void (*fp_console_setrow)(struct console_t *pcons, u32 row, int clr);
} *con = (void *) 0x09F0A39E8;

int board_early_init_f(void)
{
	printf("con: %hd %hd %hd %hd %p %u %u %u\n", con->curr_row, con->curr_col, con->cols, con->rows, con->fbbase, con->lcdsizex, con->lcdsizey, con->lcdrot);

	return 0;
}

/* Use routines provided by previous u-boot as a debug UART */
static inline void _debug_uart_init(void)
{
	// Already initialized.
}

static inline void _debug_uart_putc(int ch)
{
	// These routines are provided by previous u-boot
	static int (*t1011s_lcd_printf)(const char *fmt, ...) = (void *)0x9F00D1F0;
	static void (*t1011s_lcd_putc)(char c) = (void *)0x9F00CF0C;

	if (isprint(ch) || ch == '\n' || ch == '\r')
		t1011s_lcd_putc(ch);
	else
		t1011s_lcd_printf("\\x%x", ch);
}

DEBUG_UART_FUNCS;

void _Noreturn reset_cpu(void)
{
	void (*start_watchdog)(u32 timeout_ms) = (void *) 0x9F031CFC;

	start_watchdog(5);
	// wait for watchdog to timeout
	while(true);
}

int board_usb_init(int index, enum usb_init_type type)
{
	void (*udc_power_on)(void) = (void *) 0x9F03E9FC;

	udc_power_on();

	// Disable USB DMA engine
	writel(1, 0x20201000);

	return 0;
}

int misc_init_r(void)
{
	board_usb_init(0, 0);
	run_commandf("bdinfo; dm tree; sleep 5; fastboot 0; sleep 30; reset");
	printf("FASTBOOT\n");
	return 0;
}
