# name of your application
APPLICATION ?= node_saul

# If no BOARD is found in the environment, use this default:
BOARD ?= samr21-xpro

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT
# Tree base
RIOT_FIRMWARES_BASE ?= $(CURDIR)/../..

# Riot Application modules
USEMODULE += shell_common

# Include pyaiot modules
USEMODULE += coap_common
EXTERNAL_MODULE_DIRS += $(RIOT_FIRMWARES_BASE)/modules/coap_common
USEMODULE += coap_utils
EXTERNAL_MODULE_DIRS += $(RIOT_FIRMWARES_BASE)/modules/coap_utils
USEMODULE += coap_saul
EXTERNAL_MODULE_DIRS += $(RIOT_FIRMWARES_BASE)/modules/coap_saul
USEMODULE += coap_led
EXTERNAL_MODULE_DIRS += $(RIOT_FIRMWARES_BASE)/modules/coap_led
USEMODULE += coap_position
EXTERNAL_MODULE_DIRS += $(RIOT_FIRMWARES_BASE)/modules/coap_position
USEMODULE += schedreg
EXTERNAL_MODULE_DIRS += $(RIOT_FIRMWARES_BASE)/modules/schedreg

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Application specific definitions and includes
APPLICATION_NAME ?= "Saul\ Node"
CFLAGS += -DAPPLICATION_NAME="\"$(APPLICATION_NAME)\""

RIOT_MAKEFILES_GLOBAL_PRE += $(RIOT_FIRMWARES_BASE)/Makefile.pre
RIOT_MAKEFILES_GLOBAL_PRE += $(RIOT_FIRMWARES_BASE)/apps/Makefile.include
include $(RIOTBASE)/Makefile.include

# Set a custom channel if needed
include $(RIOTMAKE)/default-radio-settings.inc.mk
