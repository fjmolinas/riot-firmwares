#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H

#include <stdio.h>
#include "ucg.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TFT_DISPLAY
#define TFT_DISPLAY            (ucg_dev_st7735_18x128x160)
#endif
#ifndef TFT_DISPLAY_EXT
#define TFT_DISPLAY_EXT        (ucg_ext_st7735_18)
#endif
#ifndef TFT_DEV_SPI
#define TFT_DEV_SPI            (1U)
#endif
#ifndef TFT_PIN_CS
#define TFT_PIN_CS             (GPIO_PIN(PA,14))
#endif
#ifndef TFT_PIN_CD
#define TFT_PIN_CD             (GPIO_PIN(PA,15))
#endif
#ifndef TFT_PIN_RESET
#define TFT_PIN_RESET          (GPIO_PIN(PA,8))
#endif

void tft_puts(ucg_t* ucg, char* str_data, uint8_t offset_x,
                     uint8_t offset_y, uint8_t center);

void tft_print_int(ucg_t * ucg, int data, uint8_t offset_x,
                      uint8_t offset_y, uint8_t center);

ucg_t * tft_get_ptr(void);

void init_st7735_printer(ucg_t* ucg);

#ifdef __cplusplus
}
#endif

#endif
