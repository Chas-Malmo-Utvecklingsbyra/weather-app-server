export CC = gcc
C_VERSION = c99

LIBS = curl

INCLUDE_DIRS = include src include/weather-app-shared
INCLUDE_FILES = 
CFLAGS = -std=$(C_VERSION) -Wall -Wextra -Werror -Wpedantic $(addprefix -include ,$(INCLUDE_FILES)) $(addprefix -I,$(INCLUDE_DIRS)) $(addprefix -l,$(LIBS))

SRC_DIR := src
SRC_FILES := $(shell find $(SRC_DIR) -name "*.c")

export BUILD_DIR := $(CURDIR)/build
export OBJ_DIR := $(BUILD_DIR)/obj

export SCORE_DEFINES = 
DEFINES = $(SCORE_DEFINES)
DEFINES_PREFIXED = $(addprefix -D,$(DEFINES))

PROGRAM_OBJ_DIR = $(BUILD_DIR)/program_obj
BIN_DIR := $(BUILD_DIR)/bin
BIN := $(BIN_DIR)/program

PROGRAM_OBJS = $(patsubst $(SRC_DIR)/%.c,$(PROGRAM_OBJ_DIR)/%.o,$(SRC_FILES))

all: build_core $(PROGRAM_OBJS) $(BIN)
	@echo "Build done."

build_core:
	@$(MAKE) -C include/weather-app-shared/core

build_program:
	@echo "Building program ..."
	@echo "Defines:" $(DEFINES)
	@echo "Source-files to compile:" $(SRC_FILES)
	@echo "OBJS:" $(PROGRAM_OBJS)
	@echo " "

$(PROGRAM_OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $< with flags $(CFLAGS) and defines $(DEFINES_PREFIXED) to $@..."
	@mkdir -p $(dir $@)
	@$(CC) -c $< $(CFLAGS) -o $@ $(DEFINES_PREFIXED)

$(BIN): $(PROGRAM_OBJS) build_core
	@mkdir -p $(BIN_DIR)
	@$(CC) -o $@ $(shell find $(OBJ_DIR)/core -name "*.o") $(PROGRAM_OBJS) $(CFLAGS) $(DEFINES_PREFIXED)

run: $(BIN)
	@$(BIN)

clean:
	@rm -rf $(BUILD_DIR)

.PHONY: all build_core build_program clean