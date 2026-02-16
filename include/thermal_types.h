#ifndef THERMAL_TYPES_H
#define THERMAL_TYPES_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    THERMAL_OK = 0,
    THERMAL_ERR_INVALID_ARG = -1,
    THERMAL_ERR_TIMEOUT = -2,
    THERMAL_ERR_IO = -3,
    THERMAL_ERR_NOT_INIT = -4,
    THERMAL_ERR_CALIBRATION = -5,
    THERMAL_ERR_FRAME_INVALID = -6,
    THERMAL_ERR_UNSUPPORTED = -7,
    THERMAL_ERR_BUS = -8,
    THERMAL_ERR_CHECKSUM = -9,
    THERMAL_ERR_RESET = -10
} thermal_status_t;

typedef enum {
    THERMAL_TRANSPORT_I2C,
    THERMAL_TRANSPORT_SPI
} thermal_transport_type_t;

typedef struct {
    uint16_t width;
    uint16_t height;
} thermal_resolution_t;

typedef struct {
    float *data;
    thermal_resolution_t resolution;
    uint32_t timestamp;
} thermal_frame_t;

typedef struct {
    uint16_t r;
    uint16_t g;
    uint16_t b;
} rgb_t;

typedef uint16_t rgb565_t;

#define RGB565(r, g, b) ((rgb565_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3)))

#endif

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/