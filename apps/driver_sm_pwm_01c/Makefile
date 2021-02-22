# Name of your application
APPLICATION ?= tests_sm_pwm_01c

# If no BOARD is found in the environment, use this default:
BOARD ?= samr21-xpro

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT
# Tree base
TREEBASE ?= $(CURDIR)/../..

USEMODULE += progress_bar
USEMODULE += sm_pwm_01c
EXTERNAL_MODULE_DIRS += $(TREEBASE)/drivers/sm_pwm_01c

USEMODULE += test_utils_interactive_sync

RIOT_MAKEFILES_GLOBAL_PRE += $(TREEBASE)/Makefile.pre
RIOT_MAKEFILES_GLOBAL_PRE += $(TREEBASE)/apps/Makefile.include
include $(RIOTBASE)/Makefile.include
