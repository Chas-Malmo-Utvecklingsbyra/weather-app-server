CC := gcc

SRC_DIR := src

BUILD_DIR := build

CVERSION := c90

INCLUDE_DIRS := include

CFLAGS := -std=$(CVERSION) $(addprefix -I,$(INCLUDE_DIRS)) -Wall -Wextra -MMD -MP

LDFLAGS := -flto -Wl,--gc-sections

LIBS := 

SRC := $(shell find -L $(SRC_DIR) -type f -name '*.c')

OBJ := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

DEP := $(OBJ:.o=.d)

BIN := server

all: $(BIN)
	@echo "Build complete."


$(BIN): $(OBJ)
	@$(CC) $(OBJ) -o $@ $(LDFLAGS) $(LIBS)


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

clean:
	@rm -rf $(BUILD_DIR) $(BIN)

-include $(DEP)

.PHONY: all run clean
