#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "fmt.h"
#include "saul.h"
#include "saul_reg.h"
#include "net/gcoap.h"

#include "coap_utils.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

ssize_t _saul_gcoap_response(coap_pkt_t* pdu, uint8_t *buf, size_t len,
                             void *ctx, uint8_t* payload, size_t payload_len)
{
    (void)ctx;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
    if (pdu->payload_len >= payload_len) {
        memcpy(pdu->payload, payload, payload_len);
        return resp_len + payload_len;
    }
    else {
        puts("ERROR: msg buffer too small");
        return gcoap_response(pdu, buf, len,
            COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}

static ssize_t _read_saul_data_str(uint8_t *buf, uint8_t type, uint8_t subtype)
{
    /* get first sensor of <type> */
    saul_reg_t *saul = saul_reg_find_type_and_subtype(type, subtype);
    if ((saul == NULL)) {
        DEBUG("[ERROR] Unable to find sensors of type,subtype %02x, %02x\n",
              type, subtype);
        return -1;
    }

    /* read sensor data*/
    phydat_t data;
    int dim = saul_reg_read(saul, &data);
    if (dim <= 0) {
        DEBUG_PUTS("[ERROR] dim <= 0");
        return -1;
    }

    /* format data string */
    char data_str[16];
    char scale_prefix;
    size_t len;
    int8_t scale;
    /* add unit prefix for some units */
    switch (data.unit) {
        case UNIT_UNDEF:
        case UNIT_NONE:
        case UNIT_M2:
        case UNIT_M3:
        case UNIT_PERCENT:
        case UNIT_TEMP_C:
        case UNIT_TEMP_F:
        case UNIT_DBM:
            /* no string conversion */
            scale_prefix = '\0';
            scale = data.scale;
            break;
        default:
            scale = 0;
            scale_prefix = phydat_prefix_from_scale(data.scale);
    }
    len = fmt_s16_dfp(data_str, data.val[0], scale);
    data_str[len] = '\0';
    size_t p;
    if (scale_prefix) {
        p = sprintf((char*)buf, "%s %c%s", data_str,
                    scale_prefix, phydat_unit_to_str(data.unit));
    }
    else {
        p = sprintf((char*)buf, "%s %s", data_str,
                    phydat_unit_to_str(data.unit));
    }
    *(buf + p) = '\0';
    DEBUG("%s: %s\n", __FUNCTION__, buf);

    return p;
}

ssize_t saul_coap_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    uint8_t type = *((uint8_t*) ctx);
    uint8_t subtype = *((uint8_t*) ++ctx);
    uint8_t data_str[16];
    DEBUG("%s: type,subtype %02x, %02x\n", __FUNCTION__, type, subtype);
    size_t data_len = _read_saul_data_str(data_str, type, subtype);
    if (data_len <= 0 ) {
        DEBUG_PUTS("[ERROR] data_len <= 0");
        return -1;
    }
    /* Prepare COAP response */
    return _saul_gcoap_response(pdu, buf, len, ctx, data_str, data_len);
}

void saul_coap_send(void *args)
{

    uint8_t type = *((uint8_t*) args);
    uint8_t subtype = *((uint8_t*) ++args);
    uint8_t data_str[16];
    size_t data_len = _read_saul_data_str(data_str, type, subtype);
    if (data_len == 0 ) {
        return;
    }
    uint8_t response[32];
    if (type == SAUL_SENSE_TEMP) {
        sprintf((char*)&response, "%s: %s", "temperature", data_str);
    }
    else if (type == SAUL_SENSE_PRESS) {
        sprintf((char*)&response, "%s: %s", "pressure", data_str);
    }
    else if (type == SAUL_SENSE_HUM) {
        sprintf((char*)&response, "%s: %s", "humidity", data_str);
    }
    else if (type == SAUL_SENSE_LIGHT) {
        sprintf((char*)&response, "%s: %s", "illuminance", data_str);
    }
    else if (type == SAUL_SENSE_CO2) {
        sprintf((char*)&response, "%s: %s", "eco2", data_str);
    }
    else if (type == SAUL_SENSE_TVOC) {
        sprintf((char*)&response, "%s: %s", "tvoc", data_str);
    }
    else if (type == SAUL_SENSE_TEMP) {
        sprintf((char*)&response, "%s: %s", "temperature", data_str);
    }
    else if (type == SAUL_SENSE_PM) {
        if (subtype == SAUL_SENSE_PM_1) {
            sprintf((char*)&response, "%s: %s", "pm1", data_str);
        }
        else if (subtype == SAUL_SENSE_PM_2p5) {
            sprintf((char*)&response, "%s: %s", "pm2p5", data_str);
        }
        else if (subtype == SAUL_SENSE_PM_10) {
            sprintf((char*)&response, "%s: %s", "pm10", data_str);
        }
    }
    send_coap_post((uint8_t*)"/server", response);
}
