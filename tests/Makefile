.PHONY: all test clean

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror -Wpedantic -g -I.. -I../include -I../include/weather-app-shared
LDFLAGS = -lcurl -lm

# Build directory
BUILD_DIR = build
TEST_BINARIES = $(BUILD_DIR)/test_query_parameters $(BUILD_DIR)/test_route_matcher $(BUILD_DIR)/test_route_registry $(BUILD_DIR)/test_request_handler

# Source files needed for tests
QUERY_PARAMS_SRC = ../src/httpserver/request_handler/query_parameters/query_parameters.c
ROUTE_MATCHER_SRC = ../src/httpserver/request_handler/route_matcher/route_matcher.c
ROUTE_REGISTRY_SRC = ../src/httpserver/request_handler/route_registry/route_registry.c
REQUEST_HANDLER_SRC = ../src/httpserver/request_handler/request_handler.c
ROUTE_HANDLERS = ../src/httpserver/request_handler/routes/city_handler.c ../src/httpserver/request_handler/routes/weather_handler.c ../src/httpserver/request_handler/routes/frontend_handler.c

# Core library objects (built from parent makefile)
CORE_LIB_DIR = ../build/obj/core

# Collect all core library object files
CORE_OBJS = $(shell find $(CORE_LIB_DIR) -name "*.o" 2>/dev/null)

all: $(BUILD_DIR) $(TEST_BINARIES)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/test_query_parameters: test_query_parameters.c $(QUERY_PARAMS_SRC) | $(BUILD_DIR)
	@echo "Compiling test_query_parameters..."
	@$(CC) $(CFLAGS) -o $@ $^ $(CORE_OBJS) $(LDFLAGS)

$(BUILD_DIR)/test_route_matcher: test_route_matcher.c $(ROUTE_MATCHER_SRC) | $(BUILD_DIR)
	@echo "Compiling test_route_matcher..."
	@$(CC) $(CFLAGS) -o $@ $^ $(CORE_OBJS) $(LDFLAGS)

$(BUILD_DIR)/test_route_registry: test_route_registry.c $(ROUTE_REGISTRY_SRC) $(ROUTE_MATCHER_SRC) $(QUERY_PARAMS_SRC) | $(BUILD_DIR)
	@echo "Compiling test_route_registry..."
	@$(CC) $(CFLAGS) -o $@ $^ $(CORE_OBJS) $(LDFLAGS)

$(BUILD_DIR)/test_request_handler: test_request_handler.c $(REQUEST_HANDLER_SRC) $(ROUTE_REGISTRY_SRC) $(ROUTE_MATCHER_SRC) $(QUERY_PARAMS_SRC) $(ROUTE_HANDLERS) | $(BUILD_DIR)
	@echo "Compiling test_request_handler..."
	@$(CC) $(CFLAGS) -o $@ $^ $(CORE_OBJS) $(LDFLAGS)

test: all
	@echo "Running tests..."
	@$(BUILD_DIR)/test_query_parameters && echo "✓ test_query_parameters passed" || echo "✗ test_query_parameters failed"
	@$(BUILD_DIR)/test_route_matcher && echo "✓ test_route_matcher passed" || echo "✗ test_route_matcher failed"
	@$(BUILD_DIR)/test_route_registry && echo "✓ test_route_registry passed" || echo "✗ test_route_registry failed"
	@$(BUILD_DIR)/test_request_handler && echo "✓ test_request_handler passed" || echo "✗ test_request_handler failed"

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Cleaned test build artifacts"
