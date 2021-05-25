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

# Workaround nrf52 issue
DEVELHELP ?= 1

# Include pyaiot modules
USEMODULE += coap_common
USEMODULE += coap_utils
USEMODULE += saul_coap
USEMODULE += coap_position
USEMODULE += schedreg

# Add external modules
EXTERNAL_MODULE_DIRS += $(RIOT_FIRMWARES_BASE)/modules

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Application specific definitions and includes
APPLICATION_NAME ?= "Saul\ Node"
CFLAGS += -DAPPLICATION_NAME="\"$(APPLICATION_NAME)\""
CFLAGS += -DTHREAD_STACKSIZE_MAIN=THREAD_STACKSIZE_LARGE

RIOT_MAKEFILES_GLOBAL_PRE += $(RIOT_FIRMWARES_BASE)/Makefile.pre
RIOT_MAKEFILES_GLOBAL_PRE += $(RIOT_FIRMWARES_BASE)/apps/Makefile.include
include $(RIOTBASE)/Makefile.include

# Set a custom channel if needed
include $(RIOTMAKE)/default-radio-settings.inc.mk
