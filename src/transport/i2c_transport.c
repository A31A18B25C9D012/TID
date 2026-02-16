#include "thermal_transport.h"
#include "esp32_hal.h"
#include <stdio.h>
#include <string.h>

#define I2C_MAX_RETRIES 3
#define I2C_RETRY_DELAY_MS 10

static thermal_status_t i2c_init(void *hw_handle) {
    if (!hw_handle) {
        return THERMAL_ERR_INVALID_ARG;
    }
    printf("I2C transport initialized\n");
    return THERMAL_OK;
}

static thermal_status_t i2c_deinit(void *hw_handle) {
    if (!hw_handle) {
        return THERMAL_ERR_INVALID_ARG;
    }
    printf("I2C transport deinitialized\n");
    return THERMAL_OK;
}

static thermal_status_t i2c_read_reg(void *hw_handle, uint8_t dev_addr, uint16_t reg, uint8_t *data, size_t len) {
    if (!hw_handle || !data || len == 0) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    for (int retry = 0; retry < I2C_MAX_RETRIES; retry++) {
        int result = esp32_i2c_read(hw_handle, dev_addr, reg, data, len);
        if (result == 0) {
            return THERMAL_OK;
        }
        printf("I2C read retry %d\n", retry + 1);
    }
    
    printf("I2C read failed after %d retries\n", I2C_MAX_RETRIES);
    return THERMAL_ERR_IO;
}

static thermal_status_t i2c_write_reg(void *hw_handle, uint8_t dev_addr, uint16_t reg, const uint8_t *data, size_t len) {
    if (!hw_handle || !data || len == 0) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    for (int retry = 0; retry < I2C_MAX_RETRIES; retry++) {
        int result = esp32_i2c_write(hw_handle, dev_addr, reg, data, len);
        if (result == 0) {
            return THERMAL_OK;
        }
        printf("I2C write retry %d\n", retry + 1);
    }
    
    printf("I2C write failed after %d retries\n", I2C_MAX_RETRIES);
    return THERMAL_ERR_IO;
}

static thermal_status_t i2c_read_burst(void *hw_handle, uint8_t dev_addr, uint16_t start_reg, uint8_t *buffer, size_t len) {
    if (!hw_handle || !buffer || len == 0) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    for (int retry = 0; retry < I2C_MAX_RETRIES; retry++) {
        int result = esp32_i2c_read_burst(hw_handle, dev_addr, start_reg, buffer, len);
        if (result == 0) {
            return THERMAL_OK;
        }
        printf("I2C burst read retry %d\n", retry + 1);
    }
    
    printf("I2C burst read failed after %d retries\n", I2C_MAX_RETRIES);
    return THERMAL_ERR_IO;
}

thermal_status_t i2c_transport_create(thermal_transport_t *transport, void *hw_handle) {
    if (!transport || !hw_handle) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    transport->type = THERMAL_TRANSPORT_I2C;
    transport->hw_handle = hw_handle;
    transport->init = i2c_init;
    transport->deinit = i2c_deinit;
    transport->read_reg = i2c_read_reg;
    transport->write_reg = i2c_write_reg;
    transport->read_burst = i2c_read_burst;
    
    return THERMAL_OK;
}

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/