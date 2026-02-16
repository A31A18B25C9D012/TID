#include "thermal_transport.h"
#include "esp32_hal.h"
#include <stdio.h>
#include <string.h>

#define SPI_MAX_RETRIES 3
#define SPI_RETRY_DELAY_MS 10

static thermal_status_t spi_init(void *hw_handle) {
    if (!hw_handle) {
        return THERMAL_ERR_INVALID_ARG;
    }
    printf("SPI transport initialized\n");
    return THERMAL_OK;
}

static thermal_status_t spi_deinit(void *hw_handle) {
    if (!hw_handle) {
        return THERMAL_ERR_INVALID_ARG;
    }
    printf("SPI transport deinitialized\n");
    return THERMAL_OK;
}

static thermal_status_t spi_read_reg(void *hw_handle, uint8_t dev_addr, uint16_t reg, uint8_t *data, size_t len) {
    if (!hw_handle || !data || len == 0) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    for (int retry = 0; retry < SPI_MAX_RETRIES; retry++) {
        int result = esp32_spi_read(hw_handle, dev_addr, reg, data, len);
        if (result == 0) {
            return THERMAL_OK;
        }
        printf("SPI read retry %d\n", retry + 1);
    }
    
    printf("SPI read failed after %d retries\n", SPI_MAX_RETRIES);
    return THERMAL_ERR_IO;
}

static thermal_status_t spi_write_reg(void *hw_handle, uint8_t dev_addr, uint16_t reg, const uint8_t *data, size_t len) {
    if (!hw_handle || !data || len == 0) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    for (int retry = 0; retry < SPI_MAX_RETRIES; retry++) {
        int result = esp32_spi_write(hw_handle, dev_addr, reg, data, len);
        if (result == 0) {
            return THERMAL_OK;
        }
        printf("SPI write retry %d\n", retry + 1);
    }
    
    printf("SPI write failed after %d retries\n", SPI_MAX_RETRIES);
    return THERMAL_ERR_IO;
}

static thermal_status_t spi_read_burst(void *hw_handle, uint8_t dev_addr, uint16_t start_reg, uint8_t *buffer, size_t len) {
    if (!hw_handle || !buffer || len == 0) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    for (int retry = 0; retry < SPI_MAX_RETRIES; retry++) {
        int result = esp32_spi_read_burst(hw_handle, dev_addr, start_reg, buffer, len);
        if (result == 0) {
            return THERMAL_OK;
        }
        printf("SPI burst read retry %d\n", retry + 1);
    }
    
    printf("SPI burst read failed after %d retries\n", SPI_MAX_RETRIES);
    return THERMAL_ERR_IO;
}

thermal_status_t spi_transport_create(thermal_transport_t *transport, void *hw_handle) {
    if (!transport || !hw_handle) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    transport->type = THERMAL_TRANSPORT_SPI;
    transport->hw_handle = hw_handle;
    transport->init = spi_init;
    transport->deinit = spi_deinit;
    transport->read_reg = spi_read_reg;
    transport->write_reg = spi_write_reg;
    transport->read_burst = spi_read_burst;
    
    return THERMAL_OK;
}

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/