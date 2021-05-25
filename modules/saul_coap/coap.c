#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "senml/saul.h"
#include "net/gcoap.h"
#include "saul/coap.h"

#include "coap_utils.h"

#define ENABLE_DEBUG    0
#include "debug.h"

/**
 * @brief   Internal buffer size for SENML encoded messages
 */
#ifndef CONFIG_SAUL_COAP_SENML_BUF_SIZE
#define CONFIG_SAUL_COAP_SENML_BUF_SIZE     (128U)
#endif

static ssize_t _saul_sense_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx);

uint8_t saul_sense_supported[][2] = {
    {SAUL_SENSE_CO2, SAUL_CLASS_ANY},
    {SAUL_SENSE_HUM, SAUL_CLASS_ANY},
    {SAUL_SENSE_LIGHT, SAUL_CLASS_ANY},
    {SAUL_SENSE_PM, SAUL_SENSE_PM_10},
    {SAUL_SENSE_PM, SAUL_SENSE_PM_1},
    {SAUL_SENSE_PM, SAUL_SENSE_PM_2p5},
    {SAUL_SENSE_PRESS, SAUL_CLASS_ANY},
    {SAUL_SENSE_TEMP, SAUL_CLASS_ANY},
    {SAUL_SENSE_TVOC, SAUL_CLASS_ANY},
};

/* CoAP resources (alphabetical order) */
static const coap_resource_t _resources[] = {
    { "/sense/co2", COAP_GET, _saul_sense_handler, &saul_sense_supported[0][0]},
    { "/sense/hum", COAP_GET, _saul_sense_handler, &saul_sense_supported[1][0] },
    { "/sense/light", COAP_GET, _saul_sense_handler, &saul_sense_supported[2][0] },
    { "/sense/pm10", COAP_GET, _saul_sense_handler, &saul_sense_supported[3][0] },
    { "/sense/pm1", COAP_GET, _saul_sense_handler, &saul_sense_supported[4][0] },
    { "/sense/pm2.5", COAP_GET, _saul_sense_handler, &saul_sense_supported[5][0] },
    { "/sense/press", COAP_GET, _saul_sense_handler, &saul_sense_supported[6][0] },
    { "/sense/temp", COAP_GET, _saul_sense_handler, &saul_sense_supported[7][0] },
    { "/sense/tvoc", COAP_GET, _saul_sense_handler, &saul_sense_supported[8][0]},
};

static gcoap_listener_t _listener = {
    (coap_resource_t *)&_resources[0],
    sizeof(_resources) / sizeof(_resources[0]),
    NULL,
    NULL,
    NULL
};

static ssize_t _saul_sense_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    uint8_t type = *((uint8_t *)ctx);
    uint8_t subtype = *((uint8_t *)++ctx);

    /* prepare COAP response */
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
    /* attemp to find matching sensor */
    saul_reg_t *dev = saul_reg_find_type_and_subtype(type, subtype);

    if (!dev) {
        const char *err = "device not found";
        if (pdu->payload_len >= strlen(err)) {
            memcpy(pdu->payload, err, strlen(err));
            gcoap_response(pdu, buf, len, COAP_CODE_404);
            return resp_len + strlen(err);
        }
        else {
            DEBUG_PUTS("[saul_coap]: msg buffer too small")
            return gcoap_response(pdu, buf, len,
                                  COAP_CODE_INTERNAL_SERVER_ERROR);
        }
    }
    else {
        size_t payload_len = senml_saulreg_encode_json(NULL, 0, dev);
        if (payload_len <= 0) {
            DEBUG_PUTS("[saul_coap]: can't encode")
            return gcoap_response(pdu, buf, len,
                                  COAP_CODE_INTERNAL_SERVER_ERROR);
        }
        else if (pdu->payload_len >= payload_len) {
            size_t payload_len = senml_saulreg_encode_json(pdu->payload, pdu->payload_len, dev);
            return resp_len + payload_len;
        }
        else {
            DEBUG("[saul_coap]: msg buffer (size: %d) too small for payload"
                  "(size: %d)\n", pdu->payload_len, payload_len);
            return gcoap_response(pdu, buf, len,
                                  COAP_CODE_INTERNAL_SERVER_ERROR);
        }
    }
}

void saul_sense_coap_send(void *args)
{
    uint8_t type = *((uint8_t *)args);
    uint8_t subtype = *((uint8_t *)++args);

    uint8_t buf[CONFIG_SAUL_COAP_SENML_BUF_SIZE];
    saul_reg_t *dev = saul_reg_find_type_and_subtype(type, subtype);
    if (!dev) {
        return;
    }
    size_t len = senml_saulreg_encode_json(NULL, 0, dev);
    if (len > CONFIG_SAUL_COAP_SENML_BUF_SIZE) {
        return;
    }
    senml_saulreg_encode_json(buf, CONFIG_SAUL_COAP_SENML_BUF_SIZE, dev);
    send_coap_post((uint8_t *)"/server", buf);
}

void saul_sense_coap_init(void)
{
    gcoap_register_listener(&_listener);
}
