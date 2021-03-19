/*
 * Copyright (C) 2017 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdio.h>

#include "shell.h"
#include "net/nanocoap.h"
#include "net/gcoap.h"
#include "saul.h"
#include "saul_reg.h"
#include "ztimer.h"

/* RIOT firmware libraries */
#include "coap_common.h"
#include "coap_position.h"
#include "coap_saul.h"
#include "schedreg.h"

#ifdef MODULE_COAP_SUIT
#include "suit/transport/coap.h"
#include "riotboot/slot.h"
#include "coap_suit.h"
#endif

#define BEACON_SEND_INTERVAL    (30 * MS_PER_SEC)

#define MAIN_QUEUE_SIZE       (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

static uint8_t _saul_list[][2] = {
    {SAUL_SENSE_CO2, SAUL_CLASS_ANY},
    {SAUL_SENSE_HUM, SAUL_CLASS_ANY},
    {SAUL_SENSE_LIGHT, SAUL_CLASS_ANY},
    {SAUL_SENSE_PM, SAUL_SENSE_PM_10},
    {SAUL_SENSE_PM, SAUL_SENSE_PM_1},
    {SAUL_SENSE_PM, SAUL_SENSE_PM_2p5},
    {SAUL_SENSE_PRESS, SAUL_CLASS_ANY},
    {SAUL_SENSE_TEMP, SAUL_CLASS_ANY},
    {SAUL_SENSE_TVOC, SAUL_CLASS_ANY},
};

static const uint32_t _send_int[] = {
    (6*MS_PER_SEC),  /* ECO2_SEND_INTERVAL */
    (5*MS_PER_SEC),  /* HUM_SEND_INTERVAL */
    (10*MS_PER_SEC), /* LIGHT_SEND_INTERVAL */
    (5*MS_PER_SEC),  /* PM10_SEND_INTERVAL */
    (5*MS_PER_SEC),  /* PM1_SEND_INTERVAL */
    (5*MS_PER_SEC),  /* PM2.5_SEND_INTERVAL */
    (5*MS_PER_SEC),  /* PRESS_SEND_INTERVAL */
    (5*MS_PER_SEC),  /* TEMP_SEND_INTERVAL */
    (6*MS_PER_SEC),  /* TVOC_SEND_INTERVAL */
};

/* CoAP resources (alphabetical order) */
static const coap_resource_t _resources[] = {
    { "/board", COAP_GET, board_handler, NULL },
    { "/eco2", COAP_GET, saul_coap_handler, &_saul_list[0][0]},
    { "/humidity", COAP_GET, saul_coap_handler, &_saul_list[1][0] },
    { "/light", COAP_GET, saul_coap_handler, &_saul_list[2][0] },
    { "/mcu", COAP_GET, mcu_handler, NULL },
    { "/name", COAP_GET, name_handler, NULL },
    { "/os", COAP_GET, os_handler, NULL },
    { "/pm10", COAP_GET, saul_coap_handler, &_saul_list[3][0] },
    { "/pm1", COAP_GET, saul_coap_handler, &_saul_list[4][0] },
    { "/pm2.5", COAP_GET, saul_coap_handler, &_saul_list[5][0] },
    { "/position", COAP_GET, position_handler, NULL },
    { "/pressure", COAP_GET, saul_coap_handler, &_saul_list[6][0] },
#ifdef MODULE_COAP_SUIT
    /* this line adds the whole "/suit"-subtree */
    SUIT_COAP_SUBTREE,
    { "/suit_state", COAP_GET, suit_state_handler, (void*) NULL },
#endif
    { "/temperature", COAP_GET, saul_coap_handler, &_saul_list[7][0] },
    { "/tvoc", COAP_GET, saul_coap_handler, &_saul_list[8][0]},
#ifdef MODULE_COAP_SUIT
    { "/vendor", COAP_GET, vendor_handler, NULL },
    { "/version", COAP_GET, version_handler, NULL },
#endif
};

static gcoap_listener_t _listener = {
    (coap_resource_t *)&_resources[0],
    sizeof(_resources) / sizeof(_resources[0]),
    NULL,
    NULL,
    NULL
};

int main(void)
{
    puts("RIOT Saul Node application");

    /* gnrc which needs a msg queue */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    /* start coap server loop */
    gcoap_register_listener(&_listener);

#ifdef MODULE_COAP_SUIT
    printf("running from slot %u\n", riotboot_slot_current());
    riotboot_slot_print_hdr(riotboot_slot_current());
    /* start suit coap updater thread */
    suit_coap_run();
    init_suit_coap_msg_handler();
#endif
    /* init schedreg thread */
    kernel_pid_t sched_pid = init_schedreg_thread();
    /* start beacon and register */
    init_beacon_sender();
    ztimer_t beacon_ztimer;
    msg_t beacon_msg;
    schedreg_t beacon_reg = SCHEDREG_INIT(beacon_handler, NULL, &beacon_msg,
                                          &beacon_ztimer, BEACON_SEND_INTERVAL);
    schedreg_register(&beacon_reg, sched_pid);

    /* register saul sensors if there is one */
    /* TODO: a lot of wasted memory if no saul device is present... */
    ztimer_t timer[ARRAY_SIZE(_send_int)];
    msg_t msgs[ARRAY_SIZE(_send_int)];
    schedreg_t saul_reg[ARRAY_SIZE(_send_int)];

    for (uint8_t i = 0; i < ARRAY_SIZE(_send_int); i++)
    {
        schedreg_init_pid(&saul_reg[i], saul_coap_send, &_saul_list[i][0],
            &msgs[i], &timer[i], _send_int[i]);
        if (saul_reg_find_type_and_subtype(_saul_list[i][0], _saul_list[i][1])) {
            schedreg_register(&saul_reg[i], sched_pid);
        }
    }

    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
