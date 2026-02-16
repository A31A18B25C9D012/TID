#include "esp32_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    esp32_i2c_config_t config;
    uint8_t initialized;
} esp32_i2c_handle_t;

typedef struct {
    esp32_spi_config_t config;
    uint8_t initialized;
} esp32_spi_handle_t;

void *esp32_i2c_init(const esp32_i2c_config_t *config) {
    if (!config) {
        printf("ESP32 I2C: invalid config\n");
        return NULL;
    }
    
    esp32_i2c_handle_t *handle = (esp32_i2c_handle_t *)malloc(sizeof(esp32_i2c_handle_t));
    if (!handle) {
        printf("ESP32 I2C: allocation failed\n");
        return NULL;
    }
    
    memcpy(&handle->config, config, sizeof(esp32_i2c_config_t));
    handle->initialized = 1;
    
    printf("ESP32 I2C: initialized on port %u (SCL=%u, SDA=%u, freq=%u Hz)\n",
           config->port, config->scl_pin, config->sda_pin, config->freq_hz);
    
    return handle;
}

void esp32_i2c_deinit(void *handle) {
    if (!handle) {
        return;
    }
    
    esp32_i2c_handle_t *i2c_handle = (esp32_i2c_handle_t *)handle;
    
    if (i2c_handle->initialized) {
        printf("ESP32 I2C: deinitialized port %u\n", i2c_handle->config.port);
        i2c_handle->initialized = 0;
    }
    
    free(handle);
}

int esp32_i2c_read(void *handle, uint8_t dev_addr, uint16_t reg, uint8_t *data, size_t len) {
    (void)dev_addr;
    (void)reg;
    
    if (!handle || !data || len == 0) {
        return -1;
    }
    
    esp32_i2c_handle_t *i2c_handle = (esp32_i2c_handle_t *)handle;
    
    if (!i2c_handle->initialized) {
        printf("ESP32 I2C: not initialized\n");
        return -1;
    }
    
    memset(data, 0, len);
    return 0;
}

int esp32_i2c_write(void *handle, uint8_t dev_addr, uint16_t reg, const uint8_t *data, size_t len) {
    (void)dev_addr;
    (void)reg;
    
    if (!handle || !data || len == 0) {
        return -1;
    }
    
    esp32_i2c_handle_t *i2c_handle = (esp32_i2c_handle_t *)handle;
    
    if (!i2c_handle->initialized) {
        printf("ESP32 I2C: not initialized\n");
        return -1;
    }
    
    return 0;
}

int esp32_i2c_read_burst(void *handle, uint8_t dev_addr, uint16_t start_reg, uint8_t *buffer, size_t len) {
    (void)dev_addr;
    (void)start_reg;
    
    if (!handle || !buffer || len == 0) {
        return -1;
    }
    
    esp32_i2c_handle_t *i2c_handle = (esp32_i2c_handle_t *)handle;
    
    if (!i2c_handle->initialized) {
        printf("ESP32 I2C: not initialized\n");
        return -1;
    }
    
    for (size_t i = 0; i < len; i++) {
        buffer[i] = (uint8_t)(i & 0xFF);
    }
    return 0;
}

void *esp32_spi_init(const esp32_spi_config_t *config) {
    if (!config) {
        printf("ESP32 SPI: invalid config\n");
        return NULL;
    }
    
    esp32_spi_handle_t *handle = (esp32_spi_handle_t *)malloc(sizeof(esp32_spi_handle_t));
    if (!handle) {
        printf("ESP32 SPI: allocation failed\n");
        return NULL;
    }
    
    memcpy(&handle->config, config, sizeof(esp32_spi_config_t));
    handle->initialized = 1;
    
    printf("ESP32 SPI: initialized on host %u (MOSI=%u, MISO=%u, SCLK=%u, CS=%u, freq=%u Hz)\n",
           config->host, config->mosi_pin, config->miso_pin, config->sclk_pin, config->cs_pin, config->freq_hz);
    
    return handle;
}

void esp32_spi_deinit(void *handle) {
    if (!handle) {
        return;
    }
    
    esp32_spi_handle_t *spi_handle = (esp32_spi_handle_t *)handle;
    
    if (spi_handle->initialized) {
        printf("ESP32 SPI: deinitialized host %u\n", spi_handle->config.host);
        spi_handle->initialized = 0;
    }
    
    free(handle);
}

int esp32_spi_read(void *handle, uint8_t dev_addr, uint16_t reg, uint8_t *data, size_t len) {
    (void)dev_addr;
    (void)reg;
    
    if (!handle || !data || len == 0) {
        return -1;
    }
    
    esp32_spi_handle_t *spi_handle = (esp32_spi_handle_t *)handle;
    
    if (!spi_handle->initialized) {
        printf("ESP32 SPI: not initialized\n");
        return -1;
    }
    
    memset(data, 0, len);
    return 0;
}

int esp32_spi_write(void *handle, uint8_t dev_addr, uint16_t reg, const uint8_t *data, size_t len) {
    (void)dev_addr;
    (void)reg;
    
    if (!handle || !data || len == 0) {
        return -1;
    }
    
    esp32_spi_handle_t *spi_handle = (esp32_spi_handle_t *)handle;
    
    if (!spi_handle->initialized) {
        printf("ESP32 SPI: not initialized\n");
        return -1;
    }
    
    return 0;
}

int esp32_spi_read_burst(void *handle, uint8_t dev_addr, uint16_t start_reg, uint8_t *buffer, size_t len) {
    (void)dev_addr;
    (void)start_reg;
    
    if (!handle || !buffer || len == 0) {
        return -1;
    }
    
    esp32_spi_handle_t *spi_handle = (esp32_spi_handle_t *)handle;
    
    if (!spi_handle->initialized) {
        printf("ESP32 SPI: not initialized\n");
        return -1;
    }
    
    for (size_t i = 0; i < len; i++) {
        buffer[i] = (uint8_t)(i & 0xFF);
    }
    return 0;
}

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/