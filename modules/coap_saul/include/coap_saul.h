#ifndef COAP_SAUL_H
#define COAP_SAUL_H

#include <inttypes.h>
#include "net/gcoap.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Saul Coap Handler
 *
 * @param[in] ctx   SAUL_SENSE_<TYPE> to send data, should be specified in the
 *                  coap_resource_t array.
 *
 */
ssize_t saul_coap_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);

/**
 * @brief   Sends a string with sensor data of SAUL_SENSE_<TYPE> passed through
 *          args
 *
 * @param[in] args   Pointer to SAUL_SENSE_<TYPE> to send data
 *
 */
void saul_coap_send(void *args);

#ifdef __cplusplus
}
#endif

#endif /* COAP_SAUL_H */
