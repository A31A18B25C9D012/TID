#include "thermal_processing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

thermal_status_t thermal_find_minmax(const float *frame, const thermal_resolution_t *resolution, thermal_minmax_t *result) {
    if (!frame || !resolution || !result) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    size_t total_pixels = resolution->width * resolution->height;
    if (total_pixels == 0) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    result->min_temp = FLT_MAX;
    result->max_temp = -FLT_MAX;
    result->min_x = 0;
    result->min_y = 0;
    result->max_x = 0;
    result->max_y = 0;
    
    for (uint16_t y = 0; y < resolution->height; y++) {
        for (uint16_t x = 0; x < resolution->width; x++) {
            size_t index = y * resolution->width + x;
            float temp = frame[index];
            
            if (temp < result->min_temp) {
                result->min_temp = temp;
                result->min_x = x;
                result->min_y = y;
            }
            
            if (temp > result->max_temp) {
                result->max_temp = temp;
                result->max_x = x;
                result->max_y = y;
            }
        }
    }
    
    return THERMAL_OK;
}

thermal_status_t thermal_find_hotspots(const float *frame, const thermal_resolution_t *resolution, float threshold, thermal_hotspot_t *hotspots, size_t max_spots, size_t *found) {
    if (!frame || !resolution || !hotspots || !found) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    *found = 0;
    size_t total_pixels = resolution->width * resolution->height;
    
    if (total_pixels == 0 || max_spots == 0) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    for (uint16_t y = 0; y < resolution->height && *found < max_spots; y++) {
        for (uint16_t x = 0; x < resolution->width && *found < max_spots; x++) {
            size_t index = y * resolution->width + x;
            float temp = frame[index];
            
            if (temp >= threshold) {
                uint8_t is_local_max = 1;
                
                for (int dy = -1; dy <= 1 && is_local_max; dy++) {
                    for (int dx = -1; dx <= 1 && is_local_max; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        
                        int nx = (int)x + dx;
                        int ny = (int)y + dy;
                        
                        if (nx >= 0 && nx < (int)resolution->width && ny >= 0 && ny < (int)resolution->height) {
                            size_t neighbor_idx = ny * resolution->width + nx;
                            if (frame[neighbor_idx] > temp) {
                                is_local_max = 0;
                            }
                        }
                    }
                }
                
                if (is_local_max) {
                    hotspots[*found].x = x;
                    hotspots[*found].y = y;
                    hotspots[*found].temperature = temp;
                    (*found)++;
                }
            }
        }
    }
    
    return THERMAL_OK;
}

thermal_status_t thermal_interpolate_bilinear(const float *src, const thermal_resolution_t *src_res, float *dst, const thermal_resolution_t *dst_res) {
    if (!src || !src_res || !dst || !dst_res) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    if (src_res->width == 0 || src_res->height == 0 || dst_res->width == 0 || dst_res->height == 0) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    float x_ratio = (float)(src_res->width - 1) / (float)(dst_res->width - 1);
    float y_ratio = (float)(src_res->height - 1) / (float)(dst_res->height - 1);
    
    for (uint16_t y = 0; y < dst_res->height; y++) {
        for (uint16_t x = 0; x < dst_res->width; x++) {
            float src_x = x * x_ratio;
            float src_y = y * y_ratio;
            
            uint16_t x1 = (uint16_t)src_x;
            uint16_t y1 = (uint16_t)src_y;
            uint16_t x2 = (x1 + 1 < src_res->width) ? x1 + 1 : x1;
            uint16_t y2 = (y1 + 1 < src_res->height) ? y1 + 1 : y1;
            
            float dx = src_x - x1;
            float dy = src_y - y1;
            
            float q11 = src[y1 * src_res->width + x1];
            float q21 = src[y1 * src_res->width + x2];
            float q12 = src[y2 * src_res->width + x1];
            float q22 = src[y2 * src_res->width + x2];
            
            float r1 = q11 * (1.0f - dx) + q21 * dx;
            float r2 = q12 * (1.0f - dx) + q22 * dx;
            float interpolated = r1 * (1.0f - dy) + r2 * dy;
            
            dst[y * dst_res->width + x] = interpolated;
        }
    }
    
    return THERMAL_OK;
}

static int compare_floats(const void *a, const void *b) {
    float fa = *(const float *)a;
    float fb = *(const float *)b;
    if (fa < fb) return -1;
    if (fa > fb) return 1;
    return 0;
}

thermal_status_t thermal_median_filter(const float *src, const thermal_resolution_t *resolution, float *dst, uint8_t kernel_size) {
    if (!src || !resolution || !dst) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    if (kernel_size % 2 == 0 || kernel_size < 3) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    int half_kernel = kernel_size / 2;
    size_t kernel_area = kernel_size * kernel_size;
    float *window = (float *)malloc(kernel_area * sizeof(float));
    
    if (!window) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    for (uint16_t y = 0; y < resolution->height; y++) {
        for (uint16_t x = 0; x < resolution->width; x++) {
            size_t window_idx = 0;
            
            for (int ky = -half_kernel; ky <= half_kernel; ky++) {
                for (int kx = -half_kernel; kx <= half_kernel; kx++) {
                    int nx = (int)x + kx;
                    int ny = (int)y + ky;
                    
                    if (nx < 0) nx = 0;
                    if (ny < 0) ny = 0;
                    if (nx >= (int)resolution->width) nx = resolution->width - 1;
                    if (ny >= (int)resolution->height) ny = resolution->height - 1;
                    
                    window[window_idx++] = src[ny * resolution->width + nx];
                }
            }
            
            qsort(window, kernel_area, sizeof(float), compare_floats);
            dst[y * resolution->width + x] = window[kernel_area / 2];
        }
    }
    
    free(window);
    return THERMAL_OK;
}

static void temperature_to_rgb(float temp, float min_temp, float max_temp, uint8_t *r, uint8_t *g, uint8_t *b) {
    float normalized = (temp - min_temp) / (max_temp - min_temp);
    if (normalized < 0.0f) normalized = 0.0f;
    if (normalized > 1.0f) normalized = 1.0f;
    
    if (normalized < 0.25f) {
        *r = 0;
        *g = 0;
        *b = (uint8_t)(255 * (normalized / 0.25f));
    } else if (normalized < 0.5f) {
        *r = 0;
        *g = (uint8_t)(255 * ((normalized - 0.25f) / 0.25f));
        *b = 255;
    } else if (normalized < 0.75f) {
        *r = (uint8_t)(255 * ((normalized - 0.5f) / 0.25f));
        *g = 255;
        *b = (uint8_t)(255 * (1.0f - (normalized - 0.5f) / 0.25f));
    } else {
        *r = 255;
        *g = (uint8_t)(255 * (1.0f - (normalized - 0.75f) / 0.25f));
        *b = 0;
    }
}

thermal_status_t thermal_apply_colormap(const float *frame, const thermal_resolution_t *resolution, float min_temp, float max_temp, rgb565_t *output) {
    if (!frame || !resolution || !output) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    if (min_temp >= max_temp) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    size_t total_pixels = resolution->width * resolution->height;
    
    for (size_t i = 0; i < total_pixels; i++) {
        uint8_t r, g, b;
        temperature_to_rgb(frame[i], min_temp, max_temp, &r, &g, &b);
        output[i] = RGB565(r, g, b);
    }
    
    return THERMAL_OK;
}

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/