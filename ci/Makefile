APPLICATION ?= tests_$(notdir $(patsubst %/,%,$(CURDIR)))

BOARD ?= nrf52840-mdk

RIOTBASE ?= $(CURDIR)/../RIOT

# SUIT update specific stuff
USEMODULE += suit_v4
FEATURES_REQUIRED += riotboot

# Change this to 0 to not use ethos
USE_ETHOS ?= 0

ifeq (1,$(USE_ETHOS))
  GNRC_NETIF_NUMOF := 2
  USEMODULE += stdio_ethos
  USEMODULE += gnrc_uhcpc

  # ethos baudrate can be configured from make command
  ETHOS_BAUDRATE ?= 115200
  CFLAGS += -DETHOS_BAUDRATE=$(ETHOS_BAUDRATE)

  TERMDEPS += host-tools
  export TAP ?= riot0
  TERMPROG = $(RIOTTOOLS)/ethos/ethos
  TERMFLAGS = $(TAP) $(PORT)
endif

# riotboot is currently incompatible with the NRF softdevice.
# That gets automatically selected on boards that support it, unless nrfmin is
# used.
# So use that here in order to prevent the softdevice from being used.
ifeq (nrf52dk,$(BOARD))
  USEMODULE += nrfmin
endif

include $(RIOTBASE)/Makefile.include

.PHONY: host-tools

host-tools:
	$(Q)env -u CC -u CFLAGS make -C $(RIOTTOOLS)
