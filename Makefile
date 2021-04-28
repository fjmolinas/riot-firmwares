# Helper Makefile

.PHONY: all clean

all: build

# Clean all firmwares
clean:
	for app in `ls apps`; do make -C apps/$$app distclean; done

# Build all firmwares
build: init_submodules
	for app in `ls apps`; do make -C apps/$$app all; done

init_submodules:
	git submodule sync --quiet
	git submodule update --init --recursive --quiet
