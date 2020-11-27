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

#define TEMP_SEND_INTERVAL          (5*US_PER_SEC)
#define HUM_SEND_INTERVAL           (5*US_PER_SEC)
#define PRESS_SEND_INTERVAL         (5*US_PER_SEC)
#define LIGHT_SEND_INTERVAL         (10*US_PER_SEC)
#define ECO2_SEND_INTERVAL          (6*US_PER_SEC)
#define TVOC_SEND_INTERVAL          (6*US_PER_SEC)
#define BEACON_SEND_INTERVAL        (30*US_PER_SEC)

#define MAIN_QUEUE_SIZE       (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

static uint8_t _saul_list[] = {
    SAUL_SENSE_CO2,
    SAUL_SENSE_HUM,
    SAUL_SENSE_LIGHT,
    SAUL_SENSE_PRESS,
    SAUL_SENSE_TEMP,
    SAUL_SENSE_TVOC,
};

/* CoAP resources (alphabetical order) */
static const coap_resource_t _resources[] = {
    { "/board", COAP_GET, board_handler, NULL },
    { "/eco2", COAP_GET, saul_coap_handler, &_saul_list[0]},
    { "/humidity", COAP_GET, saul_coap_handler, &_saul_list[1] },
    { "/light", COAP_GET, saul_coap_handler, &_saul_list[2] },
    { "/mcu", COAP_GET, mcu_handler, NULL },
    { "/name", COAP_GET, name_handler, NULL },
    { "/os", COAP_GET, os_handler, NULL },
    { "/position", COAP_GET, position_handler, NULL },
    { "/pressure", COAP_GET, saul_coap_handler, &_saul_list[3] },
#ifdef MODULE_COAP_SUIT
    /* this line adds the whole "/suit"-subtree */
    SUIT_COAP_SUBTREE,
    { "/suit_state", COAP_GET, suit_state_handler, (void*) NULL },
#endif
    { "/temperature", COAP_GET, saul_coap_handler, &_saul_list[4]},
    { "/tvoc", COAP_GET, saul_coap_handler, &_saul_list[5]},
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
    xtimer_t beacon_xtimer;
    msg_t beacon_msg;
    schedreg_t beacon_reg = SCHEDREG_INIT(beacon_handler, NULL, &beacon_msg,
                                          &beacon_xtimer, BEACON_SEND_INTERVAL);
    schedreg_register(&beacon_reg, sched_pid);

    /* register saul sensors if there is one */
    /* TODO: a lot of wasted memory if no saul device is present... */
    xtimer_t saul_eco2_xtimer;
    msg_t saul_eco2_msg;
    schedreg_t saul_eco2_reg = SCHEDREG_INIT(saul_coap_send,
        &_saul_list[0], &saul_eco2_msg, &saul_eco2_xtimer,
        ECO2_SEND_INTERVAL);
    if (saul_reg_find_type(SAUL_SENSE_CO2)) {
        schedreg_register(&saul_eco2_reg, sched_pid);
    }

    xtimer_t saul_hum_xtimer;
    msg_t saul_hum_msg;
    schedreg_t saul_hum_reg = SCHEDREG_INIT(saul_coap_send,
        &_saul_list[1], &saul_hum_msg, &saul_hum_xtimer,
        HUM_SEND_INTERVAL);
    if (saul_reg_find_type(SAUL_SENSE_HUM)) {
        schedreg_register(&saul_hum_reg, sched_pid);
    }

    xtimer_t saul_light_xtimer;
    msg_t saul_light_msg;
    schedreg_t saul_light_reg = SCHEDREG_INIT(saul_coap_send,
        &_saul_list[2], &saul_light_msg, &saul_light_xtimer,
        LIGHT_SEND_INTERVAL);
    if (saul_reg_find_type(SAUL_SENSE_LIGHT)) {
        schedreg_register(&saul_light_reg, sched_pid);
    }

    xtimer_t saul_press_xtimer;
    msg_t saul_press_msg;
    schedreg_t saul_press_reg = SCHEDREG_INIT(saul_coap_send,
        &_saul_list[3], &saul_press_msg, &saul_press_xtimer,
        PRESS_SEND_INTERVAL);
    if (saul_reg_find_type(SAUL_SENSE_PRESS)) {
        schedreg_register(&saul_press_reg, sched_pid);
    }

    xtimer_t saul_temp_xtimer;
    msg_t saul_temp_msg;
    schedreg_t saul_temp_reg = SCHEDREG_INIT(saul_coap_send,
        &_saul_list[4], &saul_temp_msg, &saul_temp_xtimer,
        TEMP_SEND_INTERVAL);
    if (saul_reg_find_type(SAUL_SENSE_TEMP)) {
        schedreg_register(&saul_temp_reg, sched_pid);
    }

    xtimer_t saul_tvoc_xtimer;
    msg_t saul_tvoc_msg;
    schedreg_t saul_tvoc_reg = SCHEDREG_INIT(saul_coap_send,
        &_saul_list[5], &saul_tvoc_msg, &saul_tvoc_xtimer,
        TVOC_SEND_INTERVAL);
    if (saul_reg_find_type(SAUL_SENSE_TVOC)) {
        schedreg_register(&saul_tvoc_reg, sched_pid);
    }

    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
