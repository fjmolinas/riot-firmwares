/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_feather-nrf52840
 * @{
 *
 * @file
 * @brief       Board specific configuration for the Adafruit Feather nRF52840
 *              Express
 *
 * @author      Martine S. Lenders <m.lenders@fu-berlin.de>
 */

#ifndef BOARD_H
#define BOARD_H

#include "cpu.h"
#include "board_common.h"
#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    LED pin configuration
 * @{
 */
#define LED0_PIN            GPIO_PIN(1, 15)
#define LED1_PIN            GPIO_PIN(1, 10)

#define LED_PORT            (NRF_P1)
#define LED0_MASK           (1 << 15)
#define LED1_MASK           (1 << 10)
#define LED_MASK            (LED0_MASK | LED1_MASK)

#define LED0_ON             (LED_PORT->OUTSET = LED0_MASK)
#define LED0_OFF            (LED_PORT->OUTCLR = LED0_MASK)
#define LED0_TOGGLE         (LED_PORT->OUT   ^= LED0_MASK)

#define LED1_ON             (LED_PORT->OUTSET = LED1_MASK)
#define LED1_OFF            (LED_PORT->OUTCLR = LED1_MASK)
#define LED1_TOGGLE         (LED_PORT->OUT   ^= LED1_MASK)
/** @} */

/**
 * @name    Button pin configuration
 * @{
 */
#define BTN0_PIN            GPIO_PIN(1, 2)
#define BTN0_MODE           GPIO_IN_PU
/** @} */

/**
 * @name    On board bme280 module definitions
 * @{
 */
#define BMX280_PARAM_I2C_ADDR       (0x76)
/** @} */

/**
 * @name    Configuration for ST7735 feather BOARD
 * @{
 */
#define ST7735_PARAM_CS                 GPIO_PIN(0, 7)
#define ST7735_PARAM_DCX                GPIO_PIN(1, 8)
#define ST7735_PARAM_BL                 GPIO_PIN(0, 26)
#define ST7735_PARAM_INVERTED           1
#define ST7735_PARAM_NUM_LINES          160U
#define ST7735_PARAM_RGB_CHANNELS       80U
#define ST7735_OFFSET_X                 1
#define ST7735_OFFSET_Y                 26
#define BACKLIGHT_ON                    gpio_set(ST7735_PARAM_BL)
#define BACKLIGHT_OFF                   gpio_clear(ST7735_PARAM_BL)

/**
 * @name    Configuration for ILI9341 feather BOARD
 * @{
 */
#define ILI9341_PARAM_CS             GPIO_PIN(0, 26)
#define ILI9341_PARAM_DCX            GPIO_PIN(0, 27)

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
/** @} */
