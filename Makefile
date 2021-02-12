# Helper Makefile

.PHONY: all clean

all: build

# Clean all firmwares
clean:
	for fw in `ls apps`; do make -C apps/$$fw distclean; done

# Build all firmwares
build: init_submodules
	for fw in `ls apps`; do make -C apps/$$fw all; done

init_submodules:
	git submodule sync --quiet
	git submodule update --init --recursive --quiet
