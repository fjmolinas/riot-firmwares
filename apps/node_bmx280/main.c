/*
 * Copyright (C) 2017 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "shell.h"

#include "net/nanocoap.h"
#include "net/gcoap.h"

/* RIOT firmware libraries */
#include "coap_common.h"
#include "coap_position.h"
#include "coap_bmx280.h"
#include "schedreg.h"

#define BMX280_SEND_INTERVAL        (5*US_PER_SEC)
#define BEACON_SEND_INTERVAL        (30*US_PER_SEC)

#ifdef MODULE_TFT_DISPLAY
#include "tft_display.h"
#endif

#ifdef MODULE_COAP_SUIT
#include "suit/coap.h"
#include "riotboot/slot.h"
#include "coap_suit.h"
#endif

#ifndef USE_TEMP
#define USE_TEMP    true
#endif

#ifndef USE_PRES
#define USE_PRES    true
#endif

#ifndef USE_HUM
#define USE_HUM     true
#endif

static const shell_command_t shell_commands[] = {
    { NULL, NULL, NULL }
};

#define MAIN_QUEUE_SIZE       (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

/* import "ifconfig" shell command, used for printing addresses */
extern int _gnrc_netif_config(int argc, char **argv);

/* CoAP resources (alphabetical order) */
static const coap_resource_t _resources[] = {
    { "/board", COAP_GET, board_handler, NULL },
#ifdef MODULE_BME280
    { "/humidity", COAP_GET, bmx280_humidity_handler, NULL },
#endif
    { "/mcu", COAP_GET, mcu_handler, NULL },
    { "/name", COAP_GET, name_handler, NULL },
    { "/os", COAP_GET, os_handler, NULL },
    { "/position", COAP_GET, position_handler, NULL },
    { "/pressure", COAP_GET, bmx280_pressure_handler, NULL },
#ifdef MODULE_COAP_SUIT
    /* this line adds the whole "/suit"-subtree */
    SUIT_COAP_SUBTREE,
#endif
    { "/temperature", COAP_GET, bmx280_temperature_handler, NULL },
#ifdef MODULE_COAP_SUIT
    { "/vendor", COAP_GET, vendor_handler, NULL },
    { "/version", COAP_GET, version_handler, NULL },
#endif
};

static gcoap_listener_t _listener = {
    (coap_resource_t *)&_resources[0],
    sizeof(_resources) / sizeof(_resources[0]),
    NULL
};

int main(void)
{
    puts("RIOT BMX280 Node application");

    /* gnrc which needs a msg queue */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    puts("Waiting for address autoconfiguration...");
    xtimer_sleep(3);

    /* print network addresses */
    puts("Configured network interfaces:");
    _gnrc_netif_config(0, NULL);

    /* start coap server loop */
    gcoap_register_listener(&_listener);

#ifdef MODULE_COAP_SUIT
    printf("running from slot %u\n", riotboot_slot_current());
    riotboot_slot_print_hdr(riotboot_slot_current());
    /* start suit coap updater thread */
    suit_coap_run();
#endif

#ifdef MODULE_TFT_DISPLAY
    ucg_t ucg;
    /* start tft displays*/
    init_st7735_printer(&ucg);
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

    /* start bmx280 and register */
    init_bmx280_sender(USE_TEMP, USE_PRES, USE_HUM);
    xtimer_t bmx280_xtimer;
    msg_t bmx280_msg;
    schedreg_t bmx280_reg = SCHEDREG_INIT(bmx280_handler, NULL, &bmx280_msg,
                                          &bmx280_xtimer, BMX280_SEND_INTERVAL);
    schedreg_register(&bmx280_reg, sched_pid);

    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
