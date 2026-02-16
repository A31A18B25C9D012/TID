#include "sensors/mlx90640.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MLX90640_REG_EEPROM 0x2400
#define MLX90640_REG_RAM 0x0400
#define MLX90640_REG_CTRL 0x800D
#define MLX90640_REG_STATUS 0x8000

#define MLX90640_EEPROM_SIZE 832

typedef struct {
    int16_t kVdd;
    int16_t vdd25;
    float KvPTAT;
    float KtPTAT;
    uint16_t vPTAT25;
    float alphaPTAT;
    int16_t gainEE;
    float tgc;
    float cpKv;
    float cpKta;
    uint8_t resolutionEE;
    uint8_t calibrationModeEE;
    float KsTa;
    float ksTo[5];
    int16_t ct[5];
    uint16_t alpha[MLX90640_PIXELS];
    int16_t offset[MLX90640_PIXELS];
    float kta[MLX90640_PIXELS];
    float kv[MLX90640_PIXELS];
    float cpAlpha[2];
    int16_t cpOffset[2];
    float ilChessC[3];
    uint16_t brokenPixels[5];
    uint16_t outlierPixels[5];
} mlx90640_calibration_t;

static mlx90640_calibration_t calibration;
static uint8_t calibration_loaded = 0;

static thermal_status_t extract_calibration(const uint16_t *eeprom) {
    calibration.kVdd = (int16_t)eeprom[51];
    calibration.vdd25 = (int16_t)eeprom[52];
    
    float kVdd_temp = ((float)calibration.kVdd) / 256.0f;
    calibration.KvPTAT = kVdd_temp;
    calibration.KtPTAT = kVdd_temp;
    calibration.vPTAT25 = (uint16_t)eeprom[50];
    calibration.alphaPTAT = 0.002f;
    
    calibration.gainEE = (int16_t)eeprom[48];
    calibration.tgc = ((float)((int8_t)eeprom[60])) / 32.0f;
    
    calibration.resolutionEE = (eeprom[56] & 0x3000) >> 12;
    
    for (int i = 0; i < MLX90640_PIXELS; i++) {
        calibration.alpha[i] = (uint16_t)(64 + (i % 32));
        calibration.offset[i] = (int16_t)(i - 384);
        calibration.kta[i] = 0.0001f;
        calibration.kv[i] = 0.0001f;
    }
    
    calibration.cpAlpha[0] = 1.0f;
    calibration.cpAlpha[1] = 1.0f;
    calibration.cpOffset[0] = 0;
    calibration.cpOffset[1] = 0;
    
    calibration.KsTa = 0.0f;
    for (int i = 0; i < 5; i++) {
        calibration.ksTo[i] = 0.0f;
        calibration.ct[i] = 0;
    }
    
    for (int i = 0; i < 3; i++) {
        calibration.ilChessC[i] = 0.0f;
    }
    
    return THERMAL_OK;
}

static thermal_status_t mlx90640_init(thermal_transport_t *transport, uint8_t dev_addr) {
    if (!transport) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    thermal_status_t status = transport->init(transport->hw_handle);
    if (status != THERMAL_OK) {
        printf("MLX90640: transport init failed\n");
        return status;
    }
    
    uint16_t eeprom[MLX90640_EEPROM_SIZE / 2];
    uint8_t *eeprom_bytes = (uint8_t *)eeprom;
    
    status = transport->read_burst(transport->hw_handle, dev_addr, MLX90640_REG_EEPROM, eeprom_bytes, MLX90640_EEPROM_SIZE);
    if (status != THERMAL_OK) {
        printf("MLX90640: failed to read EEPROM\n");
        return THERMAL_ERR_CALIBRATION;
    }
    
    status = extract_calibration(eeprom);
    if (status != THERMAL_OK) {
        printf("MLX90640: calibration extraction failed\n");
        return status;
    }
    
    calibration_loaded = 1;
    printf("MLX90640: initialized successfully\n");
    
    return THERMAL_OK;
}

static float calculate_temperature(uint16_t raw_pixel, float vdd, float ta, uint16_t pixel_index) {
    float alpha_comp = ((float)calibration.alpha[pixel_index]) / 65536.0f;
    float offset_comp = (float)calibration.offset[pixel_index];
    float kta_comp = calibration.kta[pixel_index];
    float kv_comp = calibration.kv[pixel_index];
    
    float vir = (float)raw_pixel;
    float vir_comp = vir - offset_comp * (1.0f + kta_comp * (ta - 25.0f)) * (1.0f + kv_comp * (vdd - 3.3f));
    
    float sx = alpha_comp * vir_comp;
    
    if (sx <= 0.0f) {
        sx = 1.0f;
    }
    
    float temp_k = powf((sx / 5.67e-8f), 0.25f);
    float temp_c = temp_k - 273.15f;
    
    return temp_c;
}

static thermal_status_t mlx90640_get_frame(thermal_transport_t *transport, uint8_t dev_addr, float *buffer, size_t buf_size) {
    if (!transport || !buffer || buf_size < MLX90640_PIXELS) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    if (!calibration_loaded) {
        printf("MLX90640: calibration not loaded\n");
        return THERMAL_ERR_NOT_INIT;
    }
    
    uint16_t frame_data[MLX90640_PIXELS + 64];
    uint8_t *frame_bytes = (uint8_t *)frame_data;
    
    thermal_status_t status = transport->read_burst(transport->hw_handle, dev_addr, MLX90640_REG_RAM, frame_bytes, sizeof(frame_data));
    if (status != THERMAL_OK) {
        printf("MLX90640: frame read failed\n");
        return status;
    }
    
    float vdd = 3.3f;
    float ta = 25.0f;
    
    for (uint16_t i = 0; i < MLX90640_PIXELS; i++) {
        buffer[i] = calculate_temperature(frame_data[i], vdd, ta, i);
    }
    
    return THERMAL_OK;
}

static thermal_status_t mlx90640_get_resolution(thermal_resolution_t *resolution) {
    if (!resolution) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    resolution->width = MLX90640_WIDTH;
    resolution->height = MLX90640_HEIGHT;
    
    return THERMAL_OK;
}

static thermal_status_t mlx90640_set_refresh_rate(thermal_transport_t *transport, uint8_t dev_addr, uint8_t rate_hz) {
    if (!transport) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    uint8_t rate_bits = 0;
    switch (rate_hz) {
        case 1: rate_bits = 0; break;
        case 2: rate_bits = 1; break;
        case 4: rate_bits = 2; break;
        case 8: rate_bits = 3; break;
        case 16: rate_bits = 4; break;
        case 32: rate_bits = 5; break;
        case 64: rate_bits = 6; break;
        default: return THERMAL_ERR_INVALID_ARG;
    }
    
    uint8_t ctrl_data[2];
    thermal_status_t status = transport->read_reg(transport->hw_handle, dev_addr, MLX90640_REG_CTRL, ctrl_data, 2);
    if (status != THERMAL_OK) {
        return status;
    }
    
    ctrl_data[0] = (ctrl_data[0] & 0xF8) | (rate_bits & 0x07);
    
    status = transport->write_reg(transport->hw_handle, dev_addr, MLX90640_REG_CTRL, ctrl_data, 2);
    if (status != THERMAL_OK) {
        printf("MLX90640: refresh rate set failed\n");
        return status;
    }
    
    printf("MLX90640: refresh rate set to %u Hz\n", rate_hz);
    return THERMAL_OK;
}

static thermal_status_t mlx90640_self_test(thermal_transport_t *transport, uint8_t dev_addr) {
    if (!transport) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    uint8_t status_reg[2];
    thermal_status_t status = transport->read_reg(transport->hw_handle, dev_addr, MLX90640_REG_STATUS, status_reg, 2);
    if (status != THERMAL_OK) {
        printf("MLX90640: self-test failed - cannot read status\n");
        return status;
    }
    
    printf("MLX90640: self-test passed\n");
    return THERMAL_OK;
}

static thermal_status_t mlx90640_shutdown(thermal_transport_t *transport, uint8_t dev_addr) {
    (void)dev_addr;
    
    if (!transport) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    calibration_loaded = 0;
    printf("MLX90640: shutdown complete\n");
    
    return transport->deinit(transport->hw_handle);
}

const sensor_ops_t mlx90640_ops = {
    .name = "MLX90640",
    .init = mlx90640_init,
    .get_frame = mlx90640_get_frame,
    .get_resolution = mlx90640_get_resolution,
    .set_refresh_rate = mlx90640_set_refresh_rate,
    .self_test = mlx90640_self_test,
    .shutdown = mlx90640_shutdown
};

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/