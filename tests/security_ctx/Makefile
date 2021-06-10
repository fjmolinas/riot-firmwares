# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT
include $(RIOTBASE)/tests/Makefile.tests_common

USEMODULE += embunit
USEMODULE += security_ctx
DEFAULT_MODULE += test_utils_interactive_sync

include $(RIOTBASE)/Makefile.include
