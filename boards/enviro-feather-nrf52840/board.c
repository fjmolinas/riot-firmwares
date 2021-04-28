/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author  Martine Lenders <m.lenders@fu-berlin.de>
 */

#include "cpu.h"
#include "board.h"

#include "periph/gpio.h"
#include "kernel_defines.h"

extern void board_feather_nrf52840_init(void);

void board_init(void)
{
    board_feather_nrf52840_init();
}

/** @} */
