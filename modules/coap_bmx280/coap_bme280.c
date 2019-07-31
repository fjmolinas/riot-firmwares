#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "msg.h"
#include "periph/i2c.h"

#include "bmx280_params.h"
#include "bmx280.h"

#include "net/gcoap.h"

#include "coap_utils.h"
#include "coap_bmx280.h"

#ifdef MODULE_TFT_DISPLAY
#include "tft_display.h"
#endif

#define ENABLE_DEBUG (0)
#include "debug.h"

static bmx280_t bmx280_dev;
static uint8_t response[64] = { 0 };

static bool use_temperature = false;
static bool use_pressure = false;

#ifdef MODULE_BME280
static bool use_humidity = false;
#endif

ssize_t bmx280_temperature_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    ssize_t p = 0;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    int16_t temperature = bmx280_read_temperature(&bmx280_dev);
    bool negative = (temperature < 0);
    if (negative) {
        temperature = -temperature;
    }
    p += sprintf((char*)response, "%s%d.%d°C",
                 (negative) ? "-" : "",
                 temperature / 100, (temperature % 100) / 10);
    response[p] = '\0';
    memcpy(pdu->payload, response, p);

    return gcoap_finish(pdu, p, COAP_FORMAT_TEXT);
}

ssize_t bmx280_pressure_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    ssize_t p = 0;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    uint32_t pressure = bmx280_read_pressure(&bmx280_dev);
    p += sprintf((char*)response, "%lu.%dhPa",
                 (unsigned long)pressure / 100,
                 (int)pressure % 100);
    response[p] = '\0';
    memcpy(pdu->payload, response, p);

    return gcoap_finish(pdu, p, COAP_FORMAT_TEXT);
}

#ifdef MODULE_BME280
ssize_t bmx280_humidity_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    ssize_t p = 0;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memset(response, 0, sizeof(response));
    uint16_t humidity = bme280_read_humidity(&bmx280_dev);
    p += sprintf((char*)response, "%u.%02u%%",
            (unsigned int)(humidity / 100),
            (unsigned int)(humidity % 100));
    response[p] = '\0';
    memcpy(pdu->payload, response, p);

    return gcoap_finish(pdu, p, COAP_FORMAT_TEXT);
}
#endif

void bmx280_handler(void *args)
{
    (void)args;
    if (use_temperature) {
        ssize_t p1 = 0;
        ssize_t p2 = 0;
        int16_t temp = bmx280_read_temperature(&bmx280_dev);
        bool negative = (temp < 0);
        if (negative) {
            temp = -temp;
        }
        p1 = sprintf((char*)&response[p1], "temperature:");
        p2 = sprintf((char*)&response[p1], "%s%2d.%02d°C",
                        negative ? "-" : "",
                        temp / 100, (temp % 100) /10);
        response[p1 + p2] = '\0';
#ifdef MODULE_TFT_DISPLAY
        display_send_buf(TFT_DISPLAY_TEMP, (uint8_t*) response + p1, p2);
#endif
        send_coap_post((uint8_t*)"/server", response);
    }

    if (use_pressure) {
        ssize_t p1 = 0;
        ssize_t p2 = 0;
        uint32_t pres = bmx280_read_pressure(&bmx280_dev);
        p1 = sprintf((char*)&response[p1], "pressure:");
        p2 = sprintf((char*)&response[p1], "%lu.%dhPa",
                        (unsigned long)pres / 100,
                        (int)pres % 100);
        response[p1 + p2] = '\0';
#ifdef MODULE_TFT_DISPLAY
        display_send_buf(TFT_DISPLAY_PRES, (uint8_t*) response + p1, p2);
#endif
        send_coap_post((uint8_t*)"/server", response);
    }

#ifdef MODULE_BME280
    if (use_humidity) {
        ssize_t p1 = 0;
        ssize_t p2 = 0;
        uint16_t hum = bme280_read_humidity(&bmx280_dev);
        p1 = sprintf((char*)&response[p1], "humidity:");
        p2 = sprintf((char*)&response[p1], "%u.%02u%%",
                        (unsigned int)(hum / 100),
                        (unsigned int)(hum % 100));
        response[p1 + p2] = '\0';
#ifdef MODULE_TFT_DISPLAY
        display_send_buf(TFT_DISPLAY_HUM, (uint8_t*) response + p1, p2);
#endif
        send_coap_post((uint8_t*)"/server", response);
    }
#endif
}

void init_bmx280_sender(bool temperature, bool pressure, bool humidity)
{
    use_temperature = temperature;
    use_pressure = pressure;
#ifdef MODULE_BME280
    use_humidity = humidity;
#endif

    /* Initialize the BMX280 sensor */
    printf("+------------Initializing BMX280 sensor ------------+\n");
    int result = bmx280_init(&bmx280_dev, &bmx280_params[0]);
    if (result == -1) {
        puts("[Error] The given i2c is not enabled");
    }
    else if (result == -2) {
        puts("[Error] The sensor did not answer correctly on the given address");
    }
    else {
        printf("Initialization successful\n\n");
    }
}
