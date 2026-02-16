#ifndef THERMAL_CORE_H
#define THERMAL_CORE_H

#include "thermal_types.h"
#include "thermal_transport.h"
#include "sensors/sensor_ops.h"

typedef struct {
    thermal_transport_t *transport;
    const sensor_ops_t *sensor_ops;
    uint8_t device_addr;
    thermal_resolution_t resolution;
    uint8_t initialized;
} thermal_device_t;

thermal_status_t thermal_init(thermal_device_t *device, thermal_transport_t *transport, const sensor_ops_t *sensor_ops, uint8_t dev_addr);
thermal_status_t thermal_get_frame(thermal_device_t *device, thermal_frame_t *frame);
thermal_status_t thermal_get_resolution(thermal_device_t *device, thermal_resolution_t *resolution);
thermal_status_t thermal_set_refresh_rate(thermal_device_t *device, uint8_t rate_hz);
thermal_status_t thermal_self_test(thermal_device_t *device);
thermal_status_t thermal_shutdown(thermal_device_t *device);

#endif

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/