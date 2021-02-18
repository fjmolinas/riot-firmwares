#ifndef COAP_UTILS_H
#define COAP_UTILS_H

#include <inttypes.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_GATEWAY_ADDR
#define CONFIG_GATEWAY_ADDR      (fd00:dead:beef::1)
#endif

#ifndef CONFIG_GATEWAY_PORT
#define CONFIG_GATEWAY_PORT      (5688)
#endif

void send_coap_post(uint8_t* uri_path, uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif
