#ifndef AMG8833_H
#define AMG8833_H

#include "sensors/sensor_ops.h"

#define AMG8833_I2C_ADDR 0x69
#define AMG8833_WIDTH 8
#define AMG8833_HEIGHT 8
#define AMG8833_PIXELS (AMG8833_WIDTH * AMG8833_HEIGHT)

extern const sensor_ops_t amg8833_ops;

#endif

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/