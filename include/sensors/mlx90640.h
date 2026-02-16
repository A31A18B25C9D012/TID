#ifndef MLX90640_H
#define MLX90640_H

#include "sensors/sensor_ops.h"

#define MLX90640_I2C_ADDR 0x33
#define MLX90640_WIDTH 32
#define MLX90640_HEIGHT 24
#define MLX90640_PIXELS (MLX90640_WIDTH * MLX90640_HEIGHT)

extern const sensor_ops_t mlx90640_ops;

#endif

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/