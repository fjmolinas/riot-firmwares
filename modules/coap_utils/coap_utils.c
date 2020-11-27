#include <inttypes.h>
#include <stdlib.h>

#include "net/gcoap.h"
#include "coap_utils.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

void send_coap_post(uint8_t* uri_path, uint8_t *data)
{
    /* format destination address from string */
    ipv6_addr_t remote_addr;
    if (ipv6_addr_from_str(&remote_addr, BROKER_ADDR) == NULL) {
        DEBUG("[ERROR]: address not valid '%s'\n", BROKER_ADDR);
        return;
    }

    DEBUG("[DEBUG] utils: sending to '%s'\n", BROKER_ADDR);
    sock_udp_ep_t remote;

    remote.family = AF_INET6;
    remote.netif  = SOCK_ADDR_ANY_NETIF;
    remote.port   = BROKER_PORT;

    memcpy(&remote.addr.ipv6[0], &remote_addr.u8[0], sizeof(remote_addr.u8));

    uint8_t buf[CONFIG_GCOAP_PDU_BUF_SIZE];
    coap_pkt_t pdu;
    size_t len;
    gcoap_req_init(&pdu, &buf[0], CONFIG_GCOAP_PDU_BUF_SIZE, COAP_METHOD_POST, (char*)uri_path);
    coap_hdr_set_type(pdu.hdr, COAP_TYPE_NON);
    coap_opt_add_format(&pdu, COAP_FORMAT_TEXT);
    len = coap_opt_finish(&pdu, COAP_OPT_FINISH_PAYLOAD);

    if (pdu.payload_len >= strlen((char*)data)) {
        memcpy(pdu.payload, (char*)data, strlen((char*)data));
        len += strlen((char*)data);
    }
    else {
        puts("gcoap_cli: msg buffer too small");
    }

    DEBUG("[INFO] Sending '%s' to '%s:%i%s'\n", data, BROKER_ADDR, BROKER_PORT, uri_path);

    gcoap_req_send(&buf[0], len, &remote, NULL, NULL);
}
