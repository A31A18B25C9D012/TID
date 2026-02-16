#ifndef ESP32_HAL_H
#define ESP32_HAL_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t scl_pin;
    uint8_t sda_pin;
    uint32_t freq_hz;
    uint8_t port;
} esp32_i2c_config_t;

typedef struct {
    uint8_t mosi_pin;
    uint8_t miso_pin;
    uint8_t sclk_pin;
    uint8_t cs_pin;
    uint32_t freq_hz;
    uint8_t host;
} esp32_spi_config_t;

void *esp32_i2c_init(const esp32_i2c_config_t *config);
void esp32_i2c_deinit(void *handle);
int esp32_i2c_read(void *handle, uint8_t dev_addr, uint16_t reg, uint8_t *data, size_t len);
int esp32_i2c_write(void *handle, uint8_t dev_addr, uint16_t reg, const uint8_t *data, size_t len);
int esp32_i2c_read_burst(void *handle, uint8_t dev_addr, uint16_t start_reg, uint8_t *buffer, size_t len);

void *esp32_spi_init(const esp32_spi_config_t *config);
void esp32_spi_deinit(void *handle);
int esp32_spi_read(void *handle, uint8_t dev_addr, uint16_t reg, uint8_t *data, size_t len);
int esp32_spi_write(void *handle, uint8_t dev_addr, uint16_t reg, const uint8_t *data, size_t len);
int esp32_spi_read_burst(void *handle, uint8_t dev_addr, uint16_t start_reg, uint8_t *buffer, size_t len);

#endif

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/