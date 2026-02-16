#ifndef SENSOR_OPS_H
#define SENSOR_OPS_H

#include "thermal_types.h"
#include "thermal_transport.h"

typedef struct sensor_ops sensor_ops_t;

typedef thermal_status_t (*sensor_init_fn)(thermal_transport_t *transport, uint8_t dev_addr);
typedef thermal_status_t (*sensor_get_frame_fn)(thermal_transport_t *transport, uint8_t dev_addr, float *buffer, size_t buf_size);
typedef thermal_status_t (*sensor_get_resolution_fn)(thermal_resolution_t *resolution);
typedef thermal_status_t (*sensor_set_refresh_rate_fn)(thermal_transport_t *transport, uint8_t dev_addr, uint8_t rate_hz);
typedef thermal_status_t (*sensor_self_test_fn)(thermal_transport_t *transport, uint8_t dev_addr);
typedef thermal_status_t (*sensor_shutdown_fn)(thermal_transport_t *transport, uint8_t dev_addr);

struct sensor_ops {
    const char *name;
    sensor_init_fn init;
    sensor_get_frame_fn get_frame;
    sensor_get_resolution_fn get_resolution;
    sensor_set_refresh_rate_fn set_refresh_rate;
    sensor_self_test_fn self_test;
    sensor_shutdown_fn shutdown;
};

#endif

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/