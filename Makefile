# Makefile for ESP32 Thermal Sensor Library
# Developed by Brandon | Github; A31A18B25C9D012
# For public use and modification, see LICENSE file in the root of this repository.

CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -Iplatform/esp32 -std=c11 -O2 -DESP32_PLATFORM
LDFLAGS = -lm

SRC_DIR = src
PLATFORM_DIR = platform/esp32
EXAMPLES_DIR = example
BUILD_DIR = build
BIN_DIR = bin

SOURCES = $(SRC_DIR)/thermal_core.c \
          $(SRC_DIR)/thermal_processing.c \
          $(SRC_DIR)/transport/i2c_transport.c \
          $(SRC_DIR)/transport/spi_transport.c \
          $(SRC_DIR)/sensors/mlx90640.c \
          $(SRC_DIR)/sensors/amg8833.c \
          $(PLATFORM_DIR)/esp32_hal.c

EXAMPLE_SOURCES = $(EXAMPLES_DIR)/main.c

OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SOURCES)))
EXAMPLE_OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(EXAMPLE_SOURCES)))

TARGET = $(BIN_DIR)/thermal_example

.PHONY: all clean directories

all: directories $(TARGET)

directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

$(TARGET): $(OBJECTS) $(EXAMPLE_OBJECTS)
	$(CC) $(OBJECTS) $(EXAMPLE_OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/transport/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/sensors/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(PLATFORM_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(EXAMPLES_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Clean complete"

run: $(TARGET)
	./$(TARGET)
