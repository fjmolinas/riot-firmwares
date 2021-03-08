# name of your application
APPLICATION = border_router

# If no BOARD is found in the environment, use this default:
BOARD ?= samr21-xpro

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for 6LoWPAN border router
USEMODULE += gnrc_sixlowpan_border_router_default
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# Change this to 0 to not use ethos
UPLINK ?= ethos
# Default prefix
IPV6_PREFIX ?= 2001:db8::/64

ifeq (ethos,$(UPLINK))
  # ethos baudrate can be configured from make command
  USEMODULE += gnrc_uhcpc
  USEMODULE += stdio_ethos
  include $(CURDIR)/Makefile.ethos.conf
else ifeq (cdc-ecm,$(UPLINK))
  USEMODULE += gnrc_uhcpc
  include $(CURDIR)/Makefile.cdc-ecm.conf
else
  $(error Supported uplinks are `ethos`, `cdc-ecm`)
endif

include $(RIOTBASE)/Makefile.include

.PHONY: host-tools

host-tools:
	$(Q)env -u CC -u CFLAGS $(MAKE) -C $(RIOTTOOLS)

uhcpd-daemon: host-tools
	$(RIOTTOOLS)/uhcpd/bin/uhcpd $(TAP) $(IPV6_PREFIX) &

include $(RIOTMAKE)/default-radio-settings.inc.mk
