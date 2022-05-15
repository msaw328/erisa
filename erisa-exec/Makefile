.PHONY: all clear
.DEFAULT_GOAL := all

BUILD_DIR := ../build/erisa-exec

all: $(BUILD_DIR)/erisa-exec

# Source files
SRC := main.c

# Add the src/ prefix
SRC := $(addprefix src/, $(SRC))

## Generate object and dependency files from source files
OBJ := $(patsubst src/%.c,$(BUILD_DIR)/%.o, $(SRC))
DEP := $(patsubst src/%.c,$(BUILD_DIR)/%.d, $(SRC))

CFLAGS := -Wall -Wextra -Werror -Wno-unused -pedantic -std=c99 -ffile-prefix-map=./=/ -I../liberisa/include

include $(DEP)

# Each dependency file is generated from the source file
$(BUILD_DIR)/%.d: src/%.c
	@echo -e "[DEP] $@"
	@$(CC) -MM -MT $(patsubst src/%.c,$(BUILD_DIR)/%.o, $<) $< > $@

$(BUILD_DIR)/%.o: src/%.c
	@echo -e "[CC] $@"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/erisa-exec: $(OBJ)
	@echo -e "[LD] $@"
	@$(CC) -L../build/liberisa -lerisa $(CFLAGS) $^ -o $@

clear:
	@echo no cleaning
