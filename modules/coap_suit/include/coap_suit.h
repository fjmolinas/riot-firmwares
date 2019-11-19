#ifndef COAP_SUIT_H
#define COAP_SUIT_H

#include <inttypes.h>

#include "net/gcoap.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t vendor_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
ssize_t version_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
ssize_t suit_state_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);
void suit_msg_handler(void *args);
int init_suit_coap_msg_handler(void);
#ifdef __cplusplus
}
#endif

#endif
