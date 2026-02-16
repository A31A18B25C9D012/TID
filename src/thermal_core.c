#include "thermal_core.h"
#include <stdio.h>
#include <string.h>

thermal_status_t thermal_init(thermal_device_t *device, thermal_transport_t *transport, const sensor_ops_t *sensor_ops, uint8_t dev_addr) {
    printf("thermal_init: device=%p, transport=%p, sensor_ops=%p, dev_addr=%u\n", 
           (void*)device, (void*)transport, (const void*)sensor_ops, dev_addr);
    fflush(stdout);
    
    if (!device || !transport || !sensor_ops) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    printf("thermal_init: checking sensor ops...\n");
    fflush(stdout);
    
    if (!sensor_ops->init || !sensor_ops->get_frame || !sensor_ops->get_resolution) {
        printf("Thermal: sensor operations incomplete\n");
        return THERMAL_ERR_INVALID_ARG;
    }
    
    printf("thermal_init: setting up device structure...\n");
    fflush(stdout);
    
    device->transport = transport;
    device->sensor_ops = sensor_ops;
    device->device_addr = dev_addr;
    device->initialized = 0;
    
    printf("thermal_init: calling sensor init...\n");
    fflush(stdout);
    thermal_status_t status = sensor_ops->init(transport, dev_addr);
    printf("thermal_init: sensor init returned %d\n", status);
    fflush(stdout);
    if (status != THERMAL_OK) {
        printf("Thermal: sensor initialization failed\n");
        return status;
    }
    
    printf("thermal_init: getting resolution...\n");
    fflush(stdout);
    status = sensor_ops->get_resolution(&device->resolution);
    printf("thermal_init: get_resolution returned %d\n", status);
    fflush(stdout);
    if (status != THERMAL_OK) {
        printf("Thermal: failed to get resolution\n");
        return status;
    }
    
    printf("thermal_init: marking device as initialized...\n");
    fflush(stdout);
    device->initialized = 1;
    printf("Thermal: device initialized - %s (%ux%u)\n", 
           sensor_ops->name, device->resolution.width, device->resolution.height);
    
    printf("thermal_init: complete, returning...\n");
    fflush(stdout);
    return THERMAL_OK;
}

thermal_status_t thermal_get_frame(thermal_device_t *device, thermal_frame_t *frame) {
    if (!device || !frame || !frame->data) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    if (!device->initialized) {
        printf("Thermal: device not initialized\n");
        return THERMAL_ERR_NOT_INIT;
    }
    
    size_t expected_size = device->resolution.width * device->resolution.height;
    
    thermal_status_t status = device->sensor_ops->get_frame(
        device->transport, 
        device->device_addr, 
        frame->data, 
        expected_size
    );
    
    if (status != THERMAL_OK) {
        printf("Thermal: frame acquisition failed\n");
        return status;
    }
    
    frame->resolution.width = device->resolution.width;
    frame->resolution.height = device->resolution.height;
    
    static uint32_t frame_counter = 0;
    frame->timestamp = frame_counter++;
    
    return THERMAL_OK;
}

thermal_status_t thermal_get_resolution(thermal_device_t *device, thermal_resolution_t *resolution) {
    if (!device || !resolution) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    if (!device->initialized) {
        return THERMAL_ERR_NOT_INIT;
    }
    
    resolution->width = device->resolution.width;
    resolution->height = device->resolution.height;
    
    return THERMAL_OK;
}

thermal_status_t thermal_set_refresh_rate(thermal_device_t *device, uint8_t rate_hz) {
    if (!device) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    if (!device->initialized) {
        return THERMAL_ERR_NOT_INIT;
    }
    
    if (!device->sensor_ops->set_refresh_rate) {
        return THERMAL_ERR_UNSUPPORTED;
    }
    
    return device->sensor_ops->set_refresh_rate(device->transport, device->device_addr, rate_hz);
}

thermal_status_t thermal_self_test(thermal_device_t *device) {
    if (!device) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    if (!device->initialized) {
        return THERMAL_ERR_NOT_INIT;
    }
    
    if (!device->sensor_ops->self_test) {
        printf("Thermal: self-test not supported\n");
        return THERMAL_ERR_UNSUPPORTED;
    }
    
    return device->sensor_ops->self_test(device->transport, device->device_addr);
}

thermal_status_t thermal_shutdown(thermal_device_t *device) {
    if (!device) {
        return THERMAL_ERR_INVALID_ARG;
    }
    
    if (!device->initialized) {
        return THERMAL_ERR_NOT_INIT;
    }
    
    thermal_status_t status = THERMAL_OK;
    
    if (device->sensor_ops->shutdown) {
        status = device->sensor_ops->shutdown(device->transport, device->device_addr);
    }
    
    device->initialized = 0;
    printf("Thermal: device shutdown\n");
    
    return status;
}

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/