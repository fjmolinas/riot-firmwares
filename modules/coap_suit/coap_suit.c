#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "net/gcoap.h"

#include "coap_suit.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#ifndef NODE_SUIT_VERSION
#define NODE_SUIT_VERSION       "0000000"
#endif

#ifndef NODE_SUIT_VENDOR
#define NODE_SUIT_VENDOR        "Unknown"
#endif

ssize_t version_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    DEBUG("[DEBUG] common: replying to 'version' request\n");
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    size_t payload_len = strlen(NODE_SUIT_VERSION);
    memcpy(pdu->payload, NODE_SUIT_VERSION, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

ssize_t vendor_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    DEBUG("[DEBUG] common: replying to 'vendor' request\n");
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    size_t payload_len = strlen(NODE_SUIT_VENDOR);
    memcpy(pdu->payload, NODE_SUIT_VENDOR, payload_len);

    return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}
