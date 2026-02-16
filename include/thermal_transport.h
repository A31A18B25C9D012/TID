#ifndef THERMAL_TRANSPORT_H
#define THERMAL_TRANSPORT_H

#include "thermal_types.h"

typedef struct thermal_transport thermal_transport_t;

typedef thermal_status_t (*transport_init_fn)(void *hw_handle);
typedef thermal_status_t (*transport_deinit_fn)(void *hw_handle);
typedef thermal_status_t (*transport_read_reg_fn)(void *hw_handle, uint8_t dev_addr, uint16_t reg, uint8_t *data, size_t len);
typedef thermal_status_t (*transport_write_reg_fn)(void *hw_handle, uint8_t dev_addr, uint16_t reg, const uint8_t *data, size_t len);
typedef thermal_status_t (*transport_read_burst_fn)(void *hw_handle, uint8_t dev_addr, uint16_t start_reg, uint8_t *buffer, size_t len);

struct thermal_transport {
    thermal_transport_type_t type;
    void *hw_handle;
    transport_init_fn init;
    transport_deinit_fn deinit;
    transport_read_reg_fn read_reg;
    transport_write_reg_fn write_reg;
    transport_read_burst_fn read_burst;
};

thermal_status_t i2c_transport_create(thermal_transport_t *transport, void *hw_handle);
thermal_status_t spi_transport_create(thermal_transport_t *transport, void *hw_handle);

#endif

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/