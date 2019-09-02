#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "net/gcoap.h"

#include "coap_suit.h"
#include "coap_utils.h"

#ifdef MODULE_SUITREG
#include "suitreg.h"
#endif

#define ENABLE_DEBUG (0)
#include "debug.h"

#ifndef NODE_SUIT_VERSION
#define NODE_SUIT_VERSION       "0000000"
#endif

#ifndef NODE_SUIT_VENDOR
#define NODE_SUIT_VENDOR        "Unknown"
#endif

#define SUIT_FW_PROGRESS_CYCLE    (16U)

#define SUIT_COAP_MSG_QUEUE_SIZE    (4)

#define SUIT_IDLE                   (0xFFFF)
#define SUIT_STALE_DELAY            (10*US_PER_SEC)

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

static msg_t _suit_coap_thread_msg_queue[SUIT_COAP_MSG_QUEUE_SIZE];
static char suit_coap_thread_stack[THREAD_STACKSIZE_DEFAULT];

static uint8_t suit_state = SUIT_STATE_IDLE;

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

ssize_t suit_state_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;
    DEBUG("[DEBUG] common: replying to 'suit_state' request\n");
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    size_t payload_len = sprintf((char*) pdu->payload, "%d", suit_state);
    return gcoap_finish(pdu, payload_len, COAP_FORMAT_OCTET);
}

void *suit_coap_thread(void *args)
{
    (void) args;
    msg_init_queue(_suit_coap_thread_msg_queue, SUIT_COAP_MSG_QUEUE_SIZE);
    msg_t m;
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
                xtimer_set_msg(&timer, SUIT_STALE_DELAY, &m_tx, thread_getpid());
                break;
            case SUIT_SEQ_NR_ERROR:
                suit_state = SUIT_STATE_SEQ_NR_ERROR;
                m_tx.type = SUIT_IDLE;
                xtimer_set_msg(&timer, SUIT_STALE_DELAY, &m_tx, thread_getpid());
                break;
            case SUIT_DIGEST_START:
                suit_state = SUIT_STATE_DIGEST_START;
                break;
            case SUIT_DIGEST_ERROR:
                suit_state = SUIT_STATE_DIGEST_ERROR;
                m_tx.type = SUIT_IDLE;
                xtimer_set_msg(&timer, SUIT_STALE_DELAY, &m_tx, thread_getpid());
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
                count = count % SUIT_FW_PROGRESS_CYCLE;
                break;
            case SUIT_DOWNLOAD_ERROR:
                suit_state = SUIT_STATE_DOWNLOAD_ERROR;
                m_tx.type = SUIT_IDLE;
                xtimer_set_msg(&timer, SUIT_STALE_DELAY, &m_tx, thread_getpid());
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