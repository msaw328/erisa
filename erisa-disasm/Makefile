.PHONY: all clear
.DEFAULT_GOAL := all

# BUILD_DIR_ROOT from top level make
BUILD_DIR := $(BUILD_DIR_ROOT)/erisa-disasm

all: $(BUILD_DIR)/erisa-disasm

# Source files
SRC := main.c

# Add the src/ prefix
SRC := $(addprefix src/, $(SRC))

## Generate object and dependency files from source files
OBJ := $(patsubst src/%.c,$(BUILD_DIR)/%.o, $(SRC))
DEP := $(patsubst src/%.c,$(BUILD_DIR)/%.d, $(SRC))

include $(DEP)

# Each dependency file is generated from the source file
$(BUILD_DIR)/%.d: src/%.c
	@echo -e "[DEP] $(subst $(BUILD_DIR)/,,$@)"
	@$(CC) $(CFLAGS) -MM -MT $(patsubst src/%.c,$(BUILD_DIR)/%.o, $<) $< > $@

$(BUILD_DIR)/%.o: src/%.c
	@echo -e "[CC] $(subst $(BUILD_DIR)/,,$@)"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/erisa-disasm: $(OBJ)
	@echo -e "[LD] $(subst $(BUILD_DIR)/,,$@)"
	@$(CC) -L$(BUILD_DIR_ROOT)/liberisa/ -lerisa $(CFLAGS) $^ -o $@
