#ifndef THERMAL_PROCESSING_H
#define THERMAL_PROCESSING_H

#include "thermal_types.h"

typedef struct {
    float min_temp;
    float max_temp;
    uint16_t min_x;
    uint16_t min_y;
    uint16_t max_x;
    uint16_t max_y;
} thermal_minmax_t;

typedef struct {
    uint16_t x;
    uint16_t y;
    float temperature;
} thermal_hotspot_t;

thermal_status_t thermal_find_minmax(const float *frame, const thermal_resolution_t *resolution, thermal_minmax_t *result);
thermal_status_t thermal_find_hotspots(const float *frame, const thermal_resolution_t *resolution, float threshold, thermal_hotspot_t *hotspots, size_t max_spots, size_t *found);
thermal_status_t thermal_interpolate_bilinear(const float *src, const thermal_resolution_t *src_res, float *dst, const thermal_resolution_t *dst_res);
thermal_status_t thermal_median_filter(const float *src, const thermal_resolution_t *resolution, float *dst, uint8_t kernel_size);
thermal_status_t thermal_apply_colormap(const float *frame, const thermal_resolution_t *resolution, float min_temp, float max_temp, rgb565_t *output);

#endif

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/