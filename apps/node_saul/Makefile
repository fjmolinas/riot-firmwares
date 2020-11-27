# name of your application
APPLICATION ?= node_saul

# If no BOARD is found in the environment, use this default:
BOARD ?= samr21-xpro

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

# Include suit Update firmware, default doesn't include
USE_SUIT ?= 0

# Riot Application modules
USEMODULE += shell_common
USEMODULE += xtimer

# Include pyaiot modules
USEMODULE += coap_common
USEMODULE += coap_utils
USEMODULE += coap_saul
USEMODULE += coap_led
USEMODULE += coap_position
USEMODULE += schedreg

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Application specific definitions and includes
APPLICATION_NAME ?= "Saul\ Node"

include $(CURDIR)/../Makefile.dep
include $(CURDIR)/../Makefile.include

include $(RIOTBASE)/Makefile.include

CFLAGS += -DAPPLICATION_NAME="\"$(APPLICATION_NAME)\""
CFLAGS += -DNODE_SUIT_VENDOR="\"$(SUIT_VENDOR)\""

# Set a custom channel if needed
include $(RIOTMAKE)/default-radio-settings.inc.mk

# Change default address for CCS811
CFLAGS += -DCCS811_PARAM_I2C_ADDR=CCS811_I2C_ADDRESS_2
