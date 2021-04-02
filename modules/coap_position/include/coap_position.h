#ifndef COAP_POSITION_H
#define COAP_POSITION_H

#include <inttypes.h>

#include "net/gcoap.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_POSITION_LAT
#define CONFIG_POSITION_LAT          "45.18536"
#endif

#ifndef CONFIG_POSITION_LNG
#define CONFIG_POSITION_LNG          "5.717535"
#endif

ssize_t position_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* COAP_POSITION_H */
