# name of your application
APPLICATION ?= node_saul

# If no BOARD is found in the environment, use this default:
BOARD ?= samr21-xpro

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT
# Tree base
TREEBASE ?= $(CURDIR)/../..

# Include suit Update firmware, default doesn't include
USE_SUIT ?= 0

# Riot Application modules
USEMODULE += shell_common

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

RIOT_MAKEFILES_GLOBAL_PRE += $(TREEBASE)/Makefile.pre
RIOT_MAKEFILES_GLOBAL_PRE += $(TREEBASE)/apps/Makefile.include
include $(RIOTBASE)/Makefile.include
# Set a custom channel if needed
include $(RIOTMAKE)/default-radio-settings.inc.mk
