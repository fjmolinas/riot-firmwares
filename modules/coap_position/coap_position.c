#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "net/gcoap.h"

#include "coap_position.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static uint8_t response[64] = { 0 };

ssize_t position_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    ssize_t p = 0;
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
    p += sprintf((char*)response, "{\"lat\":%s,\"lng\":%s}",
                 CONFIG_POSITION_LAT, CONFIG_POSITION_LNG);
    response[p] = '\0';

    if (pdu->payload_len >= p) {
        memcpy(pdu->payload, response, p);
        return resp_len + p;
    }
    else {
        puts("ERROR: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}
