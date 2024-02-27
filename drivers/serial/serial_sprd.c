// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Serial driver for SpreadTrum SoCs.
 * Based on Linux driver
 *
 * Copyright (R) 2024, Yang Xiwen <forbidden405@outlook.com>
 */

#define LOG_CATEGORY UCLASS_SERIAL

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <reset.h>
#include <serial.h>

#define SERIAL_SPRD_DEFAULT_RATE 75000000

void sprd_serial_setbrg();
