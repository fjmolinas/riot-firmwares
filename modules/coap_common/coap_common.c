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

ssize_t _handler_str(coap_pkt_t* pdu, uint8_t *buf, size_t len, const char* str)
{
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
    size_t p = strlen(str);

    if (pdu->payload_len >= p) {
        memcpy(pdu->payload, str, p);
        return resp_len + p;
    }
    else {
        DEBUG_PUTS("ERROR: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}

ssize_t name_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    return _handler_str(pdu, buf, len, CONFIG_NAME_RESOURCE_STR);
}

ssize_t board_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    return _handler_str(pdu, buf, len, RIOT_BOARD);
}

ssize_t mcu_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    return _handler_str(pdu, buf, len, RIOT_MCU);
}

ssize_t os_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    const char *os = "riot";
    return _handler_str(pdu, buf, len, os);
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
