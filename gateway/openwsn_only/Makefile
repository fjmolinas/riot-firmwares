APPLICATION ?= openwsn_only

BOARD ?= nrf52840-mdk

RIOTBASE ?= $(CURDIR)/../../RIOT

RIOT_TERMINAL ?= socat

QUIET ?= 1
# DEVELHELP enabled by default for all tests, set 0 to disable
DEVELHELP ?= 0

# OpenWSN Modules
USEPKG += openwsn
USEMODULE += openwsn_openstack
USEMODULE += openwsn_cjoin
USEMODULE += openwsn_6lo_fragmentation
USEMODULE += openwsn_adaptive_msf
USEMODULE += openwsn_serial
USEMODULE += stdio_null
# Optional Modules
USEMODULE += openwsn_leds

# cjoin requires sock
USEMODULE += sock_udp

include $(RIOTBASE)/Makefile.include
