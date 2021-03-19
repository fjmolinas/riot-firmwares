APPLICATION ?= gui

USE_ETHOS ?= 1

DEVELHELP ?= 1

BOARD ?= stm32f429i-disc1

RIOTBASE ?= $(CURDIR)/../../RIOT
TREEBASE ?= $(CURDIR)/../..

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for IPv6 and UDP
USEMODULE += gnrc_ipv6_default
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_sock_udp
# Add echo module, can be removed
USEMODULE += gnrc_icmpv6_echo

USEMODULE += nanocoap_sock sock_util
USEMODULE += suit suit_transport_coap suit_storage_flashwrite
USEMODULE += suitreg
USEMODULE += progress_bar

# Riot Application modules
USEMODULE += shell_common

USEPKG += lvgl
USEMODULE += lvgl_contrib

CFLAGS += -DTHREAD_STACKSIZE_MAIN=2048

SUIT_COAP_FSROOT ?= $(TREEBASE)/coaproot

RIOT_MAKEFILES_GLOBAL_PRE += $(TREEBASE)/Makefile.pre
RIOT_MAKEFILES_GLOBAL_PRE += $(TREEBASE)/apps/Makefile.include
include $(RIOTBASE)/Makefile.include
