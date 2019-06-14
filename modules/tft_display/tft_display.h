#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H

#include <stdio.h>
#include "ucg.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
// suit related display messages
    TFT_DISPLAY_TRIGGER = 0x00,
    TFT_DISPLAY_SIGNATURE = 0x01,
    TFT_DISPLAY_END = 0x02,
    TFT_DISPLAY_SIZE_UPDATE = 0x03,
    TFT_DISPLAY_SIZE = 0x04,
// application related display messages
    TFT_DISPLAY_LED = 0xA0,
    TFT_DISPLAY_TEMP = 0xA1,
    TFT_DISPLAY_PRES = 0xA2,
    TFT_DISPLAY_HUM = 0xA3,
    TFT_DISPLAY_HELLO = 0xA4,
    TFT_DISPLAY_TVOC = 0xA5,
    TFT_DISPLAY_ECO2 = 0xA6,
};

#ifndef APPLICATION_NAME
#define APPLICATION_NAME       "TFT DISPLAY"
#endif

#ifndef NODE_SUIT_VERSION
#define NODE_SUIT_VERSION      "XXXXXXXX"
#endif

#ifndef TFT_DISPLAY
#define TFT_DISPLAY            (ucg_dev_st7735_18x128x128)
#endif
#ifndef TFT_DISPLAY_EXT
#define TFT_DISPLAY_EXT        (ucg_ext_st7735_18)
#endif

#ifdef BOARD_NUCLEO_L476RG
#ifndef TFT_DEV_SPI
#define TFT_DEV_SPI            (0U)
#endif
#ifndef TFT_PIN_CS
#define TFT_PIN_CS             (GPIO_PIN(PORT_B,6))
#endif
#ifndef TFT_PIN_CD
#define TFT_PIN_CD             (GPIO_PIN(PORT_A,9))
#endif
#ifndef TFT_PIN_RESET
#define TFT_PIN_RESET          (GPIO_PIN(PORT_C,7))
#endif
#else
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
#endif

void init_st7735_printer(ucg_t* ucg);

ucg_t* tft_get_ptr(void);

int tft_get_pid(void);

void display_send_str(uint16_t type, char* str, uint8_t len);

void display_send_val(uint16_t type, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif
