#include "thermal_core.h"
#include "thermal_processing.h"
#include "sensors/mlx90640.h"
#include "sensors/amg8833.h"
#include "esp32_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FRAME_SIZE 4096

static void print_frame_stats(const thermal_frame_t *frame) {
    thermal_minmax_t minmax;
    thermal_status_t status = thermal_find_minmax(frame->data, &frame->resolution, &minmax);
    
    if (status == THERMAL_OK) {
        printf("Frame %u: min=%.2f°C at (%u,%u), max=%.2f°C at (%u,%u)\n",
               frame->timestamp,
               minmax.min_temp, minmax.min_x, minmax.min_y,
               minmax.max_temp, minmax.max_x, minmax.max_y);
    }
}

static void find_and_print_hotspots(const thermal_frame_t *frame, float threshold) {
    printf("find_and_print_hotspots: called with threshold=%.2f\n", threshold);
    fflush(stdout);
    
    thermal_hotspot_t hotspots[10];
    size_t found = 0;
    
    printf("find_and_print_hotspots: calling thermal_find_hotspots\n");
    fflush(stdout);
    
    thermal_status_t status = thermal_find_hotspots(frame->data, &frame->resolution, threshold, hotspots, 10, &found);
    
    printf("find_and_print_hotspots: thermal_find_hotspots returned %d, found=%zu\n", status, found);
    fflush(stdout);
    
    if (status == THERMAL_OK && found > 0) {
        printf("Found %zu hotspots above %.2f°C:\n", found, threshold);
        for (size_t i = 0; i < found; i++) {
            printf("  Hotspot %zu: (%.2f°C) at (%u,%u)\n", 
                   i + 1, hotspots[i].temperature, hotspots[i].x, hotspots[i].y);
        }
    }
}

static thermal_status_t test_mlx90640_sensor(void) {
    printf("\n--- Testing MLX90640 Sensor ---\n");
    fflush(stdout);
    
    esp32_i2c_config_t i2c_config = {
        .scl_pin = 22,
        .sda_pin = 21,
        .freq_hz = 400000,
        .port = 0
    };
    
    printf("Initializing I2C hardware, Please wait.\n");
    fflush(stdout);
    void *hw_handle = esp32_i2c_init(&i2c_config);
    printf("I2C hardware handle: %p\n", hw_handle);
    fflush(stdout);
    if (!hw_handle) {
        printf("Failed to initialize I2C hardware\n");
        return THERMAL_ERR_IO;
    }
    
    printf("Creating transport, Please wait.\n");
    fflush(stdout);
    thermal_transport_t transport;
    thermal_status_t status = i2c_transport_create(&transport, hw_handle);
    printf("Transport create status: %d\n", status);
    fflush(stdout);
    if (status != THERMAL_OK) {
        printf("Failed to create transport\n");
        esp32_i2c_deinit(hw_handle);
        return status;
    }
    
    thermal_device_t device;
    status = thermal_init(&device, &transport, &mlx90640_ops, MLX90640_I2C_ADDR);
    printf("thermal_init returned status: %d\n", status);
    fflush(stdout);
    if (status != THERMAL_OK) {
        printf("Failed to initialize thermal device\n");
        esp32_i2c_deinit(hw_handle);
        return status;
    }
    
    printf("Getting resolution, Please wait.\n");
    fflush(stdout);
    thermal_resolution_t resolution;
    status = thermal_get_resolution(&device, &resolution);
    printf("thermal_get_resolution returned: %d\n", status);
    fflush(stdout);
    if (status == THERMAL_OK) {
        printf("Resolution: %ux%u pixels\n", resolution.width, resolution.height);
    }
    
    printf("Setting refresh rate, Please wait.\n");
    fflush(stdout);
    status = thermal_set_refresh_rate(&device, 8);
    printf("thermal_set_refresh_rate returned: %d\n", status);
    fflush(stdout);
    if (status == THERMAL_OK) {
        printf("Refresh rate configured\n");
    }
    
    printf("Running self-test, Please wait.\n");
    fflush(stdout);
    status = thermal_self_test(&device);
    printf("thermal_self_test returned: %d\n", status);
    fflush(stdout);
    if (status == THERMAL_OK) {
        printf("Self-test completed\n");
    }
    
    printf("Allocating frame buffer, Please wait.\n");
    fflush(stdout);
    float frame_buffer[MAX_FRAME_SIZE];
    printf("Frame buffer allocated\n");
    fflush(stdout);
    thermal_frame_t frame = {
        .data = frame_buffer,
        .resolution = {0, 0},
        .timestamp = 0
    };
    
    printf("Starting frame acquisition loop, Please wait.\n");
    fflush(stdout);
    for (int i = 0; i < 5; i++) {
        printf("Acquiring frame %d, Please wait.\n", i);
        fflush(stdout);
        status = thermal_get_frame(&device, &frame);
        printf("Frame %d status: %d\n", i, status);
        fflush(stdout);
        if (status == THERMAL_OK) {
            print_frame_stats(&frame);
            printf("Calling find_and_print_hotspots for frame %d\n", i);
            fflush(stdout);
            find_and_print_hotspots(&frame, 30.0f);
            printf("find_and_print_hotspots completed for frame %d\n", i);
            fflush(stdout);
        } else {
            printf("Frame acquisition failed\n");
        }
    }
    
    printf("Frame loop completed\n");
    fflush(stdout);
    
    printf("Setting up interpolation buffers, Please wait.\n");
    fflush(stdout);
    float interpolated_buffer[MAX_FRAME_SIZE];
    thermal_resolution_t high_res = {64, 48};
    
    printf("Calling bilinear interpolation, Please wait.\n");
    fflush(stdout);
    status = thermal_interpolate_bilinear(frame.data, &frame.resolution, interpolated_buffer, &high_res);
    printf("Interpolation returned: %d\n", status);
    fflush(stdout);
    if (status == THERMAL_OK) {
        printf("Bilinear interpolation: %ux%u -> %ux%u\n",
               frame.resolution.width, frame.resolution.height,
               high_res.width, high_res.height);
    }
    
    rgb565_t colormap[MAX_FRAME_SIZE];
    status = thermal_apply_colormap(frame.data, &frame.resolution, 20.0f, 40.0f, colormap);
    if (status == THERMAL_OK) {
        printf("Colormap applied (%zu pixels)\n", 
               (size_t)(frame.resolution.width * frame.resolution.height));
    }
    
    status = thermal_shutdown(&device);
    esp32_i2c_deinit(hw_handle);
    
    printf("MLX90640 test completed\n");
    return THERMAL_OK;
}

static thermal_status_t test_amg8833_sensor(void) {
    printf("\n--- Testing AMG8833 Sensor ---\n");
    
    esp32_i2c_config_t i2c_config = {
        .scl_pin = 22,
        .sda_pin = 21,
        .freq_hz = 400000,
        .port = 0
    };
    
    void *hw_handle = esp32_i2c_init(&i2c_config);
    if (!hw_handle) {
        printf("Failed to initialize I2C hardware\n");
        return THERMAL_ERR_IO;
    }
    
    thermal_transport_t transport;
    thermal_status_t status = i2c_transport_create(&transport, hw_handle);
    if (status != THERMAL_OK) {
        printf("Failed to create transport\n");
        esp32_i2c_deinit(hw_handle);
        return status;
    }
    
    thermal_device_t device;
    status = thermal_init(&device, &transport, &amg8833_ops, AMG8833_I2C_ADDR);
    if (status != THERMAL_OK) {
        printf("Failed to initialize thermal device\n");
        esp32_i2c_deinit(hw_handle);
        return status;
    }
    
    thermal_resolution_t resolution;
    status = thermal_get_resolution(&device, &resolution);
    if (status == THERMAL_OK) {
        printf("Resolution: %ux%u pixels\n", resolution.width, resolution.height);
    }
    
    status = thermal_set_refresh_rate(&device, 10);
    if (status == THERMAL_OK) {
        printf("Refresh rate configured\n");
    }
    
    status = thermal_self_test(&device);
    if (status == THERMAL_OK) {
        printf("Self-test completed\n");
    }
    
    float frame_buffer[MAX_FRAME_SIZE];
    thermal_frame_t frame = {
        .data = frame_buffer,
        .resolution = {0, 0},
        .timestamp = 0
    };
    
    for (int i = 0; i < 5; i++) {
        status = thermal_get_frame(&device, &frame);
        if (status == THERMAL_OK) {
            print_frame_stats(&frame);
        } else {
            printf("Frame acquisition failed\n");
        }
    }
    
    float filtered_buffer[MAX_FRAME_SIZE];
    status = thermal_median_filter(frame.data, &frame.resolution, filtered_buffer, 3);
    if (status == THERMAL_OK) {
        printf("Median filter applied (3x3 kernel)\n");
    }
    
    status = thermal_shutdown(&device);
    esp32_i2c_deinit(hw_handle);
    
    printf("AMG8833 test completed\n");
    return THERMAL_OK;
}

int main(void) {
    printf("Framework Example for TID(Thermal Imaging Driver)\nDeveloped by Brandon | Github; A31A18B25C9D012/TID\n");
    printf("-------------------------------------------------\n");
    fflush(stdout);
    
    thermal_status_t status;
    
    status = test_mlx90640_sensor();
    if (status != THERMAL_OK) {
        printf("MLX90640 test failed with status %d\n", status);
    }
    
    status = test_amg8833_sensor();
    if (status != THERMAL_OK) {
        printf("AMG8833 test failed with status %d\n", status);
    }
    
    printf("\nAll tests completed\n");
    return 0;
}

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/