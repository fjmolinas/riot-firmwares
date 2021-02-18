#ifndef COAP_SUIT_H
#define COAP_SUIT_H

#include <inttypes.h>

#include "net/gcoap.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_NODE_SUIT_VENDOR
#define CONFIG_NODE_SUIT_VENDOR     "RIOT-fp"
#endif

#ifndef CONFIG_SUIT_FW_PROGRESS_CYCLE
#define CONFIG_SUIT_FW_PROGRESS_CYCLE      (16U)
#endif

#ifndef CONFIG_SUIT_COAP_MSG_QUEUE_SIZE
#define CONFIG_SUIT_COAP_MSG_QUEUE_SIZE    (4)
#endif

#ifndef CONFIG_SUIT_STALE_DELAY
#define CONFIG_SUIT_STALE_DELAY            (10*US_PER_SEC)
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
