/*
 * Copyright (C) 2019 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       LittlevGL example application
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <string.h>

#include "lvgl/lvgl.h"
#include "lvgl_riot.h"
#include "disp_dev.h"

#include "suit/transport/coap.h"
#include "riotboot/slot.h"
#include "thread.h"
#include "net/nanocoap_sock.h"
#include "msg.h"
#include "suitreg.h"

#define COAP_INBUF_SIZE (256U)

/* Extend stacksize of nanocoap server thread */
static char _nanocoap_server_stack[THREAD_STACKSIZE_DEFAULT + THREAD_EXTRA_STACKSIZE_PRINTF];
#define NANOCOAP_SERVER_QUEUE_SIZE     (8)
static msg_t _nanocoap_server_msg_queue[NANOCOAP_SERVER_QUEUE_SIZE];

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

static void *_nanocoap_server_thread(void *arg)
{
    (void)arg;

    /* nanocoap_server uses gnrc sock which uses gnrc which needs a msg queue */
    msg_init_queue(_nanocoap_server_msg_queue, NANOCOAP_SERVER_QUEUE_SIZE);

    /* initialize nanocoap server instance */
    uint8_t buf[COAP_INBUF_SIZE];
    sock_udp_ep_t local = { .port=COAP_PORT, .family=AF_INET6 };
    nanocoap_server(&local, buf, sizeof(buf));

    return NULL;
}


#define CPU_LABEL_COLOR     "FF0000"
#define MEM_LABEL_COLOR     "0000FF"
#define CHART_POINT_NUM     100

/* Must be lower than LVGL_INACTIVITY_PERIOD_MS for autorefresh */
#define REFR_TIME           200

static lv_task_t *_update_suit_bar;
static lv_obj_t * _suit_bar;

typedef struct {
    uint32_t image_size;
    uint32_t image_size_written;
} suit_image_progress_t;

static void suit_progress_bar(lv_task_t *param)
{
    (void)param;
    suit_image_progress_t* image_progress = (suit_image_progress_t*) param->user_data;
    lv_bar_set_value(_suit_bar, (100*image_progress->image_size_written)/(image_progress->image_size), LV_ANIM_OFF);
    /* Force a wakeup of lvgl when each task is called: this ensures an activity
       is triggered and wakes up lvgl during the next LVGL_INACTIVITY_PERIOD ms */
    lvgl_wakeup();

}

void suit_update_create(void* arg)
{
    _suit_bar = lv_bar_create(lv_scr_act(), NULL);
    lv_obj_set_size(_suit_bar, 200, 20);
    lv_obj_align(_suit_bar, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_value(_suit_bar, 0, LV_ANIM_OFF);

    /* Refresh the chart and label manually at first */
    _update_suit_bar = lv_task_create(suit_progress_bar, REFR_TIME, LV_TASK_PRIO_HIGH, arg);
    lv_task_set_repeat_count(_update_suit_bar, -1);
    suit_progress_bar(_update_suit_bar);
}

int main(void)
{
    suit_image_progress_t image_progress = { 0 };
    /* Enable backlight */
    disp_dev_backlight_on();
    lvgl_start();
    /* Create the system monitor widget */
    suit_update_create(&image_progress);

    /* start suit coap updater thread */
    suit_coap_run();
    /* start nanocoap server thread */
    thread_create(_nanocoap_server_stack, sizeof(_nanocoap_server_stack),
                  THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST,
                  _nanocoap_server_thread, NULL, "nanocoap server");

    /* the shell contains commands that receive packets via GNRC and thus
       needs a msg queue */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    suitreg_t entry = SUITREG_INIT_PID(SUITREG_TYPE_STATUS | SUITREG_TYPE_ERROR, thread_getpid());
    suitreg_register(&entry);

    msg_t m;
    while (1) {
        msg_receive(&m);
        switch(m.type) {
            case SUIT_DOWNLOAD_START:
                image_progress.image_size = m.content.value;
                break;
            case SUIT_DOWNLOAD_PROGRESS:
                image_progress.image_size_written = m.content.value;
                break;
            case SUIT_TRIGGER:
            case SUIT_SIGNATURE_START:
            case SUIT_SIGNATURE_ERROR:
            case SUIT_SEQ_NR_ERROR:
            case SUIT_DIGEST_START:
            case SUIT_DIGEST_ERROR:
            case SUIT_REBOOT:
            case SUIT_DOWNLOAD_ERROR:
            default:
                puts("Unknown message received");
                break;
        }
    }

    return 0;
}
