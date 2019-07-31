#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "msg.h"
#include "periph/i2c.h"

#include "ccs811_params.h"
#include "ccs811.h"

#include "net/gcoap.h"

#include "coap_utils.h"
#include "coap_ccs811.h"

#ifdef MODULE_TFT_DISPLAY
#include "tft_display.h"
#endif

#define ENABLE_DEBUG (0)
#include "debug.h"

#define I2C_DEVICE           (0)

static ccs811_t ccs811_dev;
static uint8_t response[32] = { 0 };

static bool use_eco2 = false;
static bool use_tvoc = false;

ssize_t ccs811_eco2_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    if (ccs811_data_ready(&ccs811_dev) != CCS811_OK) {
        sprintf((char*)response, "0ppm");
    }
    else {
        uint16_t eco2;
        ccs811_read_iaq(&ccs811_dev, NULL, &eco2, NULL, NULL);
        sprintf((char*)response, "%ippm", eco2);
    }
    size_t payload_len = sizeof(response);
    memcpy(pdu->payload, response, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

ssize_t ccs811_tvoc_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    if (ccs811_data_ready(&ccs811_dev) != CCS811_OK) {
        sprintf((char*)response, "0ppb");
    }
    else {
        uint16_t tvoc;
        ccs811_read_iaq(&ccs811_dev, &tvoc, NULL, NULL, NULL);
        sprintf((char*)response, "%ippb", tvoc);
    }
    size_t payload_len = sizeof(response);
    memcpy(pdu->payload, response, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

void ccs811_handler(void *args)
{
    (void) args;

    if (use_eco2) {
        ssize_t p1 = 0;
        ssize_t p2 = 0;
        uint16_t eco2;
        ccs811_read_iaq(&ccs811_dev, NULL, &eco2, NULL, NULL);
        p1 = sprintf((char*)&response[p1], "eco2:");
        p2 = sprintf((char*)&response[p1], "%.4ippm", eco2);
        response[p1 + p2] = '\0';
        send_coap_post((uint8_t*)"/server", response);
#ifdef MODULE_TFT_DISPLAY
        display_send_buf(TFT_DISPLAY_ECO2, (uint8_t*) (response + p1), p2);
#endif
    }

    if (use_tvoc) {
        ssize_t p1 = 0;
        ssize_t p2 = 0;
        uint16_t tvoc;
        ccs811_read_iaq(&ccs811_dev, &tvoc, NULL, NULL, NULL);
        p1 = sprintf((char*)&response[p1], "tvoc:");
        p2 = sprintf((char*)&response[p1], "%.4ippb", tvoc);
        response[p1 + p2] = '\0';
        send_coap_post((uint8_t*)"/server", response);
#ifdef MODULE_TFT_DISPLAY
        display_send_buf(TFT_DISPLAY_TVOC, (uint8_t*) (response + p1), p2);
#endif
    }
}

void init_ccs811_sender(bool eco2, bool tvoc)
{
    use_eco2 = eco2;
    use_tvoc = tvoc;

    /* Initialize the CCS811 sensor */
    printf("+------------Initializing CCS811 sensor ------------+\n");
    int result = ccs811_init(&ccs811_dev, &ccs811_params[0]);
    if (result != 0) {
        puts("[Error] Cannot initialize CCS811 sensor");
    }
    else {
        printf("Initialization successful\n\n");
    }
}
