#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fmt.h"
#include "luid.h"
#include "thread.h"
#include "net/ieee802154.h"
#include "net/gcoap.h"

#include "coap_common.h"
#include "coap_utils.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static char uid[IEEE802154_LONG_ADDRESS_LEN * 2];

ssize_t name_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    DEBUG("[DEBUG] common: replying to 'name' request\n");
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
    size_t p = strlen(CONFIG_NAME_RESOURCE_STR);

    if (pdu->payload_len >= p) {
        memcpy(pdu->payload, CONFIG_NAME_RESOURCE_STR, p);
        return resp_len + p;
    }
    else {
        puts("ERROR: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}

ssize_t board_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    DEBUG("[DEBUG] common: replying to 'board' request\n");
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
    size_t p = strlen(RIOT_BOARD);

    if (pdu->payload_len >= p) {
        memcpy(pdu->payload, RIOT_BOARD, p);
        return resp_len + p;
    }
    else {
        puts("ERROR: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}

ssize_t mcu_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    DEBUG("[DEBUG] common: replying to 'mcu' request\n");
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
    const char *mcu = RIOT_MCU;
    size_t p = strlen(RIOT_MCU);


    if (pdu->payload_len >= p) {
        memcpy(pdu->payload, mcu, p);
        return resp_len + p;
    }
    else {
        puts("ERROR: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}

ssize_t os_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    DEBUG("[DEBUG] common: replying to 'os' request\n");
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
    const char *os = "riot";
    size_t p = strlen("riot");

    if (pdu->payload_len >= p) {
        memcpy(pdu->payload, os, p);
        return resp_len + p;
    }
    else {
        puts("ERROR: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}

void beacon_handler(void *arg)
{
    (void) arg;
    size_t msg_len = strlen("alive:") + (IEEE802154_LONG_ADDRESS_LEN * 2);
    char alive_msg[msg_len];
    snprintf(alive_msg, msg_len, "alive:%s", uid);
    /* Schedule next transmission */
    send_coap_post((uint8_t*)"/alive", (uint8_t*)alive_msg);
}

void init_beacon_sender(void)
{
    uint8_t addr[IEEE802154_LONG_ADDRESS_LEN];
    luid_get(addr, IEEE802154_LONG_ADDRESS_LEN);
    fmt_bytes_hex(uid, addr, IEEE802154_LONG_ADDRESS_LEN);
    size_t msg_len = strlen("reset:") + (IEEE802154_LONG_ADDRESS_LEN * 2);
    char reset_msg[msg_len];
    snprintf(reset_msg, msg_len, "reset:%s", uid);
    /* Schedule next transmission */
    send_coap_post((uint8_t*)"/reset", (uint8_t*)reset_msg);
}
