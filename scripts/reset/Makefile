# Fake application to call make
RIOTBASE ?= $(CURDIR)/../../RIOT
APPLICATION = firmware
BOARD ?= native

# Add suit_v4 and riotboot to use suit targets
USEMODULE += suit_v4
FEATURES_REQUIRED += riotboot

include $(RIOTBASE)/Makefile.include
