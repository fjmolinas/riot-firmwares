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

#define TFT_DISPLAY_QUEUE_SIZE    (8)

static msg_t _tft_display_msg_queue[TFT_DISPLAY_QUEUE_SIZE];
static char tft_display_stack[THREAD_STACKSIZE_DEFAULT];

static gpio_t pins[] = {
    [UCG_PIN_CS] = TFT_PIN_CS,
    [UCG_PIN_CD] = TFT_PIN_CD,
    [UCG_PIN_RST] = TFT_PIN_RESET
};

static uint32_t pins_enabled = (
    (1 << UCG_PIN_CS) +
    (1 << UCG_PIN_CD) +
    (1 << UCG_PIN_RST)
    );

static volatile ucg_t * ucg_ptr;
static volatile int tft_display_pid;

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

static void _draw_riotboot(ucg_t* ucg)
{
#ifdef MODULE_RIOTBOOT_SLOT
    // Running Slot
    ucg_SetFontPosCenter(ucg);
    ucg_SetFont(ucg, ucg_font_profont15_mr);
    tft_print_int(ucg, riotboot_slot_current(), 117, 118, 1);

    // Draw Frame For slot Info
    ucg_SetColor(ucg, 1, 0, 0, 0);
    ucg_SetColor(ucg, 0, 255, 255, 255);
    ucg_DrawRFrame(ucg, 108, 108, 18, 18, 4);

    // Draw Frame For Version Info
    ucg_DrawRFrame(ucg, 2, 108, 104, 18, 4);
    ucg_SetFont(ucg, ucg_font_profont12_mr);
    ucg_SetFontPosTop(ucg);
    char buffer [32];
    sprintf(buffer, "VER: %s", NODE_SUIT_VERSION);
    tft_puts(ucg, buffer, 52, 112, 1);
#else
    (void) ucg;
#endif
}

static void _draw_app_name(ucg_t* ucg)
{
    ucg_SetFontPosTop(ucg);
    ucg_SetFont(ucg, ucg_font_profont12_mr);
    tft_puts(ucg, (char* ) APPLICATION_NAME, 63, 0, 1);
}

static void _clear_data_area(ucg_t* ucg)
{
    ucg_SetColor(ucg, 0, 0, 0, 0);
    ucg_DrawBox(ucg, 0, 66, 128, 40);
    ucg_SetColor(ucg, 0, 255, 255, 255);
}

ucg_t * tft_get_ptr(void)
{
    return (ucg_t *) ucg_ptr;
}

int tft_get_pid(void)
{
    return (int) tft_display_pid;
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

void *tft_display_thread(void *args)
{
    (void) args;

    msg_init_queue(_tft_display_msg_queue, TFT_DISPLAY_QUEUE_SIZE);

    uint8_t update = 0;
    uint32_t fw_size = 0;
    msg_t m;

    while (1) {
        msg_receive(&m);
        switch(m.type)
        {
            case TFT_DISPLAY_LED:
                if(!update)
                {
                    ucg_SetFontPosCenter(tft_get_ptr());
                    ucg_SetFont(tft_get_ptr(), ucg_font_profont17_mr);
                    if (m.content.value)
                    {
                        tft_puts(tft_get_ptr(), "LED ON ", 64, 86, 1);
                    }
                    else
                    {
                        tft_puts(tft_get_ptr(), "LED OFF", 64, 86, 1);
                    }
                    ucg_SetFontPosTop(tft_get_ptr());
                }
                break;
            case TFT_DISPLAY_TEMP:
                if(!update)
                {
                    ucg_SetFont(tft_get_ptr(), ucg_font_profont12_mr);
                    tft_puts(tft_get_ptr(), "TEMPERATURE", 63, 70, 1);

                    ucg_SetFont(tft_get_ptr(), ucg_font_profont17_mr);
                    tft_print_int(tft_get_ptr(), m.content.value, 65, 86, 1);
                }
                break;
            case TFT_DISPLAY_TRIGGER:
                update = 1;
                _clear_data_area(tft_get_ptr());
                ucg_SetFont(tft_get_ptr(), ucg_font_profont12_mr);
                tft_puts(tft_get_ptr(), "UPDATE", 63, 70, 1);
                tft_puts(tft_get_ptr(), "STARTING", 63, 84, 1);
                break;
            case TFT_DISPLAY_SIGNATURE:
                DEBUG("[TFT]: drawing signature validation msg.\n");
                _clear_data_area(tft_get_ptr());
                ucg_SetFont(tft_get_ptr(), ucg_font_profont12_mr);
                tft_puts(tft_get_ptr(), "VERIFYING", 63, 70, 1);
                tft_puts(tft_get_ptr(), "SIGNATURE", 63, 84, 1);
                break;
            case TFT_DISPLAY_END:
                _clear_data_area(tft_get_ptr());
                ucg_SetFont(tft_get_ptr(), ucg_font_profont12_mr);
                tft_puts(tft_get_ptr(), "UPDATE", 63, 70, 1);
                tft_puts(tft_get_ptr(), "FINALIZED", 63, 84, 1);
                break;
            case TFT_DISPLAY_SIZE:
                _clear_data_area(tft_get_ptr());
                ucg_SetFont(tft_get_ptr(), ucg_font_profont12_mr);
                tft_puts(tft_get_ptr(), "UPDATING", 63, 70, 1);
                ucg_SetColor(tft_get_ptr(), 0, 255, 255, 255);
                ucg_DrawFrame(tft_get_ptr(), 12, 86, 104, 18);
                fw_size = m.content.value;
                break;
            case TFT_DISPLAY_SIZE_UPDATE:
                ucg_SetColor(tft_get_ptr(), 0, 255, 255, 255);
                ucg_DrawBox(tft_get_ptr(), 14, 88,
                            (100*m.content.value)/ fw_size, 14);
                break;
            default:
                break;
        }
    }
    return NULL;
}

void init_st7735_printer(ucg_t * ucg)
{
    ucg_ptr = ucg;

    _init_st7735(ucg);

    _draw_riot_logo(ucg, 16, 16);
    _draw_app_name(ucg);
    _draw_riotboot(ucg);

    tft_display_pid = thread_create(tft_display_stack, sizeof(tft_display_stack),
                                         THREAD_PRIORITY_MAIN - 1,
                                         THREAD_CREATE_STACKTEST, tft_display_thread,
                                         NULL, "tft_display thread");
}