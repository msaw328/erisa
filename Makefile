# Main target
ERISA_LIB := build/liberisa/liberisa.so

# Binaries
ERISA_BINS := build/erisa-exec/erisa-exec build/erisa-disasm/erisa-disasm

.PHONY: all clear $(ERISA_LIB) $(ERISA_BINS)
.DEFAULT_GOAL := all

all: $(ERISA_LIB) $(ERISA_BINS)

$(ERISA_BINS): $(ERISA_LIB)

# Main build directory
BUILD_DIR_REL := build
export BUILD_DIR_ROOT := $(abspath $(BUILD_DIR_REL))

# Main C compiler flags - all binaries need public liberisa headers, including the library itself
# Submakes may append to those flags as necessary
export CFLAGS := -Wall -Wextra -Werror -Wno-unused -Wno-unused-parameter -pedantic -std=c99 -ffile-prefix-map=./=/ -I$(abspath ./liberisa/include)

build:
	mkdir -p $(BUILD_DIR_ROOT)/liberisa $(BUILD_DIR_ROOT)/erisa-exec/ $(BUILD_DIR_ROOT)/erisa-disasm/

clear:
	@echo -e "[RM] $(BUILD_DIR_REL)"
	@$(MAKE) -C liberisa clear
	@rm $(BUILD_DIR_REL) -rf

build/liberisa/liberisa.so: build
	@$(MAKE) -C liberisa

build/erisa-exec/erisa-exec: build
	@$(MAKE) -C erisa-exec

build/erisa-disasm/erisa-disasm: build
	@$(MAKE) -C erisa-disasm
