#include <errno.h>
#include <stdlib.h>

#include "periph/gpio.h"
#include "periph/spi.h"

#include "xtimer.h"
#include "riotboot/slot.h"

#include "logo.h"
#include "tft_display.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/**
 * @brief   RIOT-OS pin maping of Ucglib pin numbers to RIOT-OS GPIO pins.
 * @note    To minimize the overhead, you can implement an alternative for
 *          ucg_com_riotos_hw_spi.
 */
static gpio_t pins[] = {
    [UCG_PIN_CS] = TFT_PIN_CS,
    [UCG_PIN_CD] = TFT_PIN_CD,
    [UCG_PIN_RST] = TFT_PIN_RESET
};

/**
 * @brief   Bit mapping to indicate which pins are set.
 */
static uint32_t pins_enabled = (
    (1 << UCG_PIN_CS) +
    (1 << UCG_PIN_CD) +
    (1 << UCG_PIN_RST)
    );

static volatile ucg_t * ucg_ptr;

static void _init_st7735(ucg_t * ucg)
{
    /* Initialize to SPI */
    DEBUG("Initializing to SPI.\n");
    ucg_SetPins(ucg, pins, pins_enabled);
    ucg_SetDevice(ucg, SPI_DEV(TFT_DEV_SPI));

    /* Initialize the display */
    DEBUG("Initializing display.\n");
    ucg_Init(ucg, TFT_DISPLAY, TFT_DISPLAY_EXT, ucg_com_riotos_hw_spi);

    /* Initial Screen Setup*/
    ucg_ClearScreen(ucg);
    ucg_SetFontMode(ucg, UCG_FONT_MODE_SOLID);
}

static void _draw_riot_logo(ucg_t* ucg, uint16_t start_x, uint16_t start_y)
{
    for (int y = 0; y < 48 ; y++)
    {
        for (int x = 0; x < 96; x++)
        {
            uint32_t offset = (x + (y * 96)) * 3;
            ucg_SetColor(ucg, 0, logo[offset + 2], logo[offset + 1], logo[offset + 0]);
            ucg_DrawPixel(ucg, x + start_x, y + start_y);
        }
    }
}

ucg_t * tft_get_ptr(void)
{
    return (ucg_t *) ucg_ptr;
}

void tft_puts(ucg_t * ucg, char* str_data, uint8_t offset_x,
                      uint8_t offset_y, uint8_t center)
{
    if(center)
    {
        uint8_t width = ucg_GetStrWidth(tft_get_ptr(), str_data);
        offset_x = (width/2 < (offset_x)) ? (offset_x - width/2) : 0;
    }
    ucg_SetColor(ucg, 1, 0, 0, 0);
    ucg_SetColor(ucg, 0, 255, 255, 255);
    ucg_DrawString(ucg, offset_x, offset_y, 0, str_data);
}

void tft_print_int(ucg_t * ucg, int data, uint8_t offset_x,
                      uint8_t offset_y, uint8_t center)
{
    char buffer [8];
    sprintf(buffer, "%i", data);
    tft_puts(ucg, buffer, offset_x, offset_y, center);
}

void init_st7735_printer(ucg_t * ucg)
{
    ucg_ptr = ucg;

    _init_st7735(ucg);

    _draw_riot_logo(ucg, 16, 12);

    ucg_SetFontPosTop(ucg);
    ucg_SetFont(ucg, ucg_font_profont12_mr);
    tft_puts(ucg, (char* ) APPLICATION_NAME_TFT, 63, 0, 1);

#ifdef MODULE_RIOTBOOT_SLOT
    ucg_SetFont(ucg, ucg_font_profont10_mr);
    tft_puts(ucg, "RUNNING FROM SLOT", 63, 64, 1);

    ucg_SetFont(ucg, ucg_font_profont17_mr);
    tft_print_int(ucg, riotboot_slot_current(), 65, 76, 1);
#endif
}