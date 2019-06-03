APPLICATION ?= tests_$(notdir $(patsubst %/,%,$(CURDIR)))

BOARD ?= samr21-xpro

RIOTBASE ?= $(CURDIR)/../../RIOT

# SUIT update specific stuff
USEMODULE += suit_v4
FEATURES_REQUIRED += riotboot

# Change this to 0 to not use ethos
USE_ETHOS ?= 1

ifeq (1,$(USE_ETHOS))
TAP ?= tap0
IPV6_PREFIX ?= 2001:db8::/64
ifeq (,$(filter native,$(BOARD)))
  GNRC_NETIF_NUMOF := 2

# ethos baudrate can be configured from make command
  ETHOS_BAUDRATE ?= 115200
  CFLAGS += -DETHOS_BAUDRATE=$(ETHOS_BAUDRATE) -DUSE_ETHOS_FOR_STDIO

  TERMDEPS += host-tools
  TERMPROG ?= sudo sh $(RIOTTOOLS)/ethos/start_network.sh
  TERMFLAGS ?= $(PORT) $(TAP) $(IPV6_PREFIX)
endif
endif

include $(RIOTBASE)/Makefile.include

.PHONY: host-tools

host-tools:
	$(Q)env -u CC -u CFLAGS make -C $(RIOTTOOLS)
