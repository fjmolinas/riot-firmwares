#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "net/gcoap.h"
#include "ztimer.h"

#include "coap_suit.h"
#include "coap_utils.h"

#include "riotboot/slot.h"

#include "suitreg.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define SUIT_IDLE                   (0xFFFF)

enum {
    SUIT_STATE_IDLE = 0x00,            /**< nothing to declare */
    SUIT_STATE_TRIGGER,                /**< trigger has been received */
    SUIT_STATE_SIGNATURE_START,        /**< manifest signature validation will start */
    SUIT_STATE_SIGNATURE_ERROR,        /**< manifest signature error */
    SUIT_STATE_SIGNATURE_END,          /**< manifest signature end */
    SUIT_STATE_SEQ_NR_ERROR,           /**< manifest invalid sequence number */
    SUIT_STATE_DOWNLOAD_START,         /**< new image download will start */
    SUIT_STATE_DOWNLOAD_PROGRESS,      /**< image download progress report */
    SUIT_STATE_DOWNLOAD_ERROR,         /**< image download error */
    SUIT_STATE_DIGEST_START,           /**< digest validation will start */
    SUIT_STATE_DIGEST_ERROR,           /**< digest validation error */
    SUIT_STATE_DOWNLOAD_END,           /**< image ended with success */
    SUIT_STATE_REBOOT,                 /**< suit is going to reboot device*/
};

static msg_t _suit_coap_thread_msg_queue[CONFIG_SUIT_COAP_MSG_QUEUE_SIZE];
static char suit_coap_thread_stack[THREAD_STACKSIZE_DEFAULT];

static uint8_t suit_state = SUIT_STATE_IDLE;

ssize_t version_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    DEBUG("[DEBUG] common: replying to 'version' request\n");
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_OCTET);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
    const riotboot_hdr_t* hdr = riotboot_slot_get_hdr(riotboot_slot_current());
    uint8_t aux[12];
    size_t p = sprintf((char*) aux, "%"PRIu32"", hdr->version);

    if (pdu->payload_len >= p) {
        memcpy(pdu->payload, aux, p);
        return resp_len + p;
    }
    else {
        puts("ERROR: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}

ssize_t vendor_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    DEBUG("[DEBUG] common: replying to 'vendor' request\n");
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
    size_t p = strlen(CONFIG_NODE_SUIT_VENDOR);

    if (pdu->payload_len >= p) {
        memcpy(pdu->payload, CONFIG_NODE_SUIT_VENDOR, p);
        return resp_len + p;
    }
    else {
        puts("ERROR: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}

ssize_t suit_state_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    DEBUG("[DEBUG] common: replying to 'suit_state' request\n");
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_OCTET);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
    uint8_t aux[12];
    size_t p = sprintf((char*) aux, "%d", suit_state);

    if (pdu->payload_len >= p) {
        memcpy(pdu->payload, aux, p);
        return resp_len + p;
    }
    else {
        puts("ERROR: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}

void *suit_coap_thread(void *args)
{
    (void) args;
    msg_init_queue(_suit_coap_thread_msg_queue, CONFIG_SUIT_COAP_MSG_QUEUE_SIZE);
    ztimer_t timer;
    msg_t m, m_tx;
    size_t p = 0;
    uint8_t response[64];
    uint8_t count = 0;
    uint32_t fw_size = 0;

    ssize_t len = sprintf((char*)&response[0], "suit_state: %d", suit_state);
    response[len] = '\0';
    send_coap_post((uint8_t*)"/server", response);

    suitreg_t entry = SUITREG_INIT_PID(SUITREG_TYPE_STATUS | SUITREG_TYPE_ERROR, thread_getpid());
    suitreg_register(&entry);

    while (1) {
        msg_receive(&m);
        switch(m.type) {
            case SUIT_TRIGGER:
                suit_state = SUIT_STATE_TRIGGER;
                break;
            case SUIT_SIGNATURE_START:
                suit_state = SUIT_STATE_SIGNATURE_START;
                break;
            case SUIT_SIGNATURE_ERROR:
                suit_state = SUIT_STATE_SIGNATURE_ERROR;
                m_tx.type = SUIT_IDLE;
                ztimer_set_msg(ZTIMER_USEC, &timer, CONFIG_SUIT_STALE_DELAY, &m_tx, thread_getpid());
                break;
            case SUIT_SEQ_NR_ERROR:
                suit_state = SUIT_STATE_SEQ_NR_ERROR;
                m_tx.type = SUIT_IDLE;
                ztimer_set_msg(ZTIMER_USEC, &timer, CONFIG_SUIT_STALE_DELAY, &m_tx, thread_getpid());
                break;
            case SUIT_DIGEST_START:
                suit_state = SUIT_STATE_DIGEST_START;
                break;
            case SUIT_DIGEST_ERROR:
                suit_state = SUIT_STATE_DIGEST_ERROR;
                m_tx.type = SUIT_IDLE;
                ztimer_set_msg(ZTIMER_USEC, &timer, CONFIG_SUIT_STALE_DELAY, &m_tx, thread_getpid());
                break;
            case SUIT_REBOOT:
                suit_state = SUIT_STATE_DOWNLOAD_END;
                break;
            case SUIT_DOWNLOAD_START:
                suit_state = SUIT_STATE_DOWNLOAD_START;
                fw_size = m.content.value;
                count = 0;
                break;
            case SUIT_DOWNLOAD_PROGRESS:
                if (count == 0) {
                    p = sprintf((char*)&response[p], "dwnld: ");
                    p += sprintf((char*)&response[p], "%ld",
                                ((100*m.content.value)/ fw_size));
                    response[p] = '\0';
                    send_coap_post((uint8_t*)"/server", response);
                }
                count++;
                count = count % CONFIG_SUIT_FW_PROGRESS_CYCLE;
                break;
            case SUIT_DOWNLOAD_ERROR:
                suit_state = SUIT_STATE_DOWNLOAD_ERROR;
                m_tx.type = SUIT_IDLE;
                ztimer_set_msg(ZTIMER_USEC, &timer, CONFIG_SUIT_STALE_DELAY, &m_tx, thread_getpid());
                break;
            case SUIT_IDLE:
                suit_state = SUIT_STATE_IDLE;
                break;
            default:
                DEBUG("Unknown message received");
                break;
        }
        if (m.type != SUIT_DOWNLOAD_PROGRESS) {
            ssize_t len = sprintf((char*)&response[0], "suit_state: %d", suit_state);
            response[len] = '\0';
            send_coap_post((uint8_t*)"/server", response);
        }
    }
    return NULL;
}

int init_suit_coap_msg_handler(void)
{
    int suit_coap_msg_pid = thread_create(suit_coap_thread_stack, sizeof(suit_coap_thread_stack),
                  THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST, suit_coap_thread,
                  NULL, "suit_coap_msg thread");
    if (suit_coap_msg_pid == -EINVAL || suit_coap_msg_pid == -EOVERFLOW) {
        puts("Error: failed to create suit coap msg thread, exiting\n");
        return suit_coap_msg_pid;
    }
    else {
        puts("Successfully created suit coap msg thread !\n");
        return suit_coap_msg_pid;
    }
}
