/*
 * Copyright (C) 2017 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdio.h>
#include "msg.h"
#include "xtimer.h"
#include "shell.h"

#include "net/nanocoap.h"
#include "net/gcoap.h"

#include "coap_common.h"
#include "coap_led.h"

#ifdef MODULE_COAP_SUIT
#include "suit/coap.h"
#include "riotboot/slot.h"
#include "coap_suit.h"
#endif

#ifdef MODULE_TFT_DISPLAY
#include "tft_display.h"
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
    { "/led", COAP_GET | COAP_POST | COAP_PUT, led_handler, NULL },
    { "/mcu", COAP_GET, mcu_handler, NULL },
    { "/name", COAP_GET, name_handler, NULL },
    { "/os", COAP_GET, os_handler, NULL },
#ifdef MODULE_COAP_SUIT
    /* this line adds the whole "/suit"-subtree */
    SUIT_COAP_SUBTREE,
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
    puts("RIOT LED Node application");

    /* gnrc which needs a msg queue */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    puts("Waiting for address autoconfiguration...");
    xtimer_sleep(3);

    /* print network addresses */
    puts("Configured network interfaces:");
    _gnrc_netif_config(0, NULL);

#ifdef MODULE_TFT_DISPLAY
    ucg_t ucg;
    /* start tft displays*/
    init_st7735_printer(&ucg);
#endif

    /* start coap server loop */
    gcoap_register_listener(&_listener);
    init_beacon_sender();

#ifdef MODULE_COAP_SUIT
    printf("running from slot %u\n", riotboot_slot_current());
    riotboot_slot_print_hdr(riotboot_slot_current());
    /* start suit coap updater thread */
    suit_coap_run();
#endif

    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
