# Name of your application
APPLICATION ?= tests_sm_pwm_01c

# If no BOARD is found in the environment, use this default:
BOARD ?= samr21-xpro

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

USEMODULE += progress_bar
USEMODULE += sm_pwm_01c

USEMODULE += test_utils_interactive_sync

EXTERNAL_MODULE_DIRS += $(CURDIR)/../../modules/sm_pwm_01c


include $(RIOTBASE)/Makefile.include
