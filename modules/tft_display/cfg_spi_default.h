/*
 * Copyright (C) 2016-2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Custom spi configuration for nrf52840-mdk
 */

#if defined(BOARD_NRF52840_MDK)

#ifndef CFG_SPI_DEFAULT_H
#define CFG_SPI_DEFAULT_H

#include "periph_cpu.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    SPI configuration
 * @{
 */
static const spi_conf_t spi_config[] = {
    {
        .dev  = NRF_SPI0,
        .sclk = GPIO_PIN(0, 28),
        .mosi = GPIO_PIN(0, 30),
        .miso = GPIO_PIN(0, 29),
    }
};

#define SPI_NUMOF           (sizeof(spi_config) / sizeof(spi_config[0]))
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* CFG_SPI_DEFAULT_H */
/** @} */

#endif