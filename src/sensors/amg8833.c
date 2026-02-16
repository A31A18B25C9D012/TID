#include "sensors/amg8833.h"
#include <stdio.h>
#include <string.h>

#define AMG8833_REG_POWER 0x00
#define AMG8833_REG_RESET 0x01
#define AMG8833_REG_FRAMERATE 0x02
#define AMG8833_REG_INT_CTRL 0x03
#define AMG8833_REG_STATUS 0x04
#define AMG8833_REG_PIXEL_BASE 0x80
#define AMG8833_REG_THERMISTOR 0x0E

#define AMG8833_POWER_NORMAL 0x00
#define AMG8833_POWER_SLEEP 0x10
#define AMG8833_RESET_FLAG 0x30
#define AMG8833_FRAMERATE_10HZ 0x00
#define AMG8833_FRAMERATE_1HZ 0x01

typedef struct {
    float offset_correction;
    float gain_correction;
    float thermistor_coefficient;
} amg8833_calibration_t;

static amg8833_calibration_t calibration = {
    .offset_correction = 0.0f,
    .gain_correction = 1.0f,
    .thermistor_coefficient = 0.0625f
};

static thermal_status_t amg8833_init(thermal_transport_t *transport, uint8_t dev_addr) {
    if (!transport) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    thermal_status_t status = transport->init(transport->hw_handle);
    if (status != THERMAL_OK) {
        printf("AMG8833: transport init failed\n");
        return status;
    }
    
    uint8_t power_mode = AMG8833_POWER_NORMAL;
    status = transport->write_reg(transport->hw_handle, dev_addr, AMG8833_REG_POWER, &power_mode, 1);
    if (status != THERMAL_OK) {
        printf("AMG8833: power mode set failed\n");
        return status;
    }
    
    uint8_t reset = AMG8833_RESET_FLAG;
    status = transport->write_reg(transport->hw_handle, dev_addr, AMG8833_REG_RESET, &reset, 1);
    if (status != THERMAL_OK) {
        printf("AMG8833: reset failed\n");
        return status;
    }
    
    uint8_t framerate = AMG8833_FRAMERATE_10HZ;
    status = transport->write_reg(transport->hw_handle, dev_addr, AMG8833_REG_FRAMERATE, &framerate, 1);
    if (status != THERMAL_OK) {
        printf("AMG8833: framerate set failed\n");
        return status;
    }
    
    printf("AMG8833: initialized successfully\n");
    return THERMAL_OK;
}

static float convert_pixel_to_celsius(int16_t raw_value) {
    float temp_c = (float)raw_value * 0.25f;
    temp_c = (temp_c + calibration.offset_correction) * calibration.gain_correction;
    return temp_c;
}

static thermal_status_t amg8833_get_frame(thermal_transport_t *transport, uint8_t dev_addr, float *buffer, size_t buf_size) {
    if (!transport || !buffer || buf_size < AMG8833_PIXELS) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    uint8_t pixel_data[AMG8833_PIXELS * 2];
    
    thermal_status_t status = transport->read_burst(transport->hw_handle, dev_addr, AMG8833_REG_PIXEL_BASE, pixel_data, sizeof(pixel_data));
    if (status != THERMAL_OK) {
        printf("AMG8833: frame read failed\n");
        return status;
    }
    
    for (uint16_t i = 0; i < AMG8833_PIXELS; i++) {
        int16_t raw_value = (int16_t)(pixel_data[i * 2] | (pixel_data[i * 2 + 1] << 8));
        
        if (raw_value & 0x800) {
            raw_value |= 0xF000;
        }
        
        buffer[i] = convert_pixel_to_celsius(raw_value);
    }
    
    return THERMAL_OK;
}

static thermal_status_t amg8833_get_resolution(thermal_resolution_t *resolution) {
    if (!resolution) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    resolution->width = AMG8833_WIDTH;
    resolution->height = AMG8833_HEIGHT;
    
    return THERMAL_OK;
}

static thermal_status_t amg8833_set_refresh_rate(thermal_transport_t *transport, uint8_t dev_addr, uint8_t rate_hz) {
    if (!transport) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    uint8_t framerate;
    if (rate_hz <= 1) {
        framerate = AMG8833_FRAMERATE_1HZ;
    } else {
        framerate = AMG8833_FRAMERATE_10HZ;
    }
    
    thermal_status_t status = transport->write_reg(transport->hw_handle, dev_addr, AMG8833_REG_FRAMERATE, &framerate, 1);
    if (status != THERMAL_OK) {
        printf("AMG8833: refresh rate set failed\n");
        return status;
    }
    
    printf("AMG8833: refresh rate set to %s\n", framerate == AMG8833_FRAMERATE_1HZ ? "1Hz" : "10Hz");
    return THERMAL_OK;
}

static thermal_status_t amg8833_self_test(thermal_transport_t *transport, uint8_t dev_addr) {
    if (!transport) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    uint8_t status_reg;
    thermal_status_t status = transport->read_reg(transport->hw_handle, dev_addr, AMG8833_REG_STATUS, &status_reg, 1);
    if (status != THERMAL_OK) {
        printf("AMG8833: self-test failed - cannot read status\n");
        return status;
    }
    
    uint8_t thermistor_data[2];
    status = transport->read_reg(transport->hw_handle, dev_addr, AMG8833_REG_THERMISTOR, thermistor_data, 2);
    if (status != THERMAL_OK) {
        printf("AMG8833: self-test failed - cannot read thermistor\n");
        return status;
    }
    
    int16_t thermistor_raw = (int16_t)(thermistor_data[0] | (thermistor_data[1] << 8));
    if (thermistor_raw & 0x800) {
        thermistor_raw |= 0xF000;
    }
    
    float thermistor_temp = (float)thermistor_raw * calibration.thermistor_coefficient;
    printf("AMG8833: self-test passed, thermistor=%.2f°C\n", thermistor_temp);
    
    return THERMAL_OK;
}

static thermal_status_t amg8833_shutdown(thermal_transport_t *transport, uint8_t dev_addr) {
    if (!transport) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    uint8_t power_mode = AMG8833_POWER_SLEEP;
    thermal_status_t status = transport->write_reg(transport->hw_handle, dev_addr, AMG8833_REG_POWER, &power_mode, 1);
    if (status != THERMAL_OK) {
        printf("AMG8833: shutdown failed\n");
        return status;
    }
    
    printf("AMG8833: shutdown complete\n");
    return transport->deinit(transport->hw_handle);
}

const sensor_ops_t amg8833_ops = {
    .name = "AMG8833",
    .init = amg8833_init,
    .get_frame = amg8833_get_frame,
    .get_resolution = amg8833_get_resolution,
    .set_refresh_rate = amg8833_set_refresh_rate,
    .self_test = amg8833_self_test,
    .shutdown = amg8833_shutdown
};

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/