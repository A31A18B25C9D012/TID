#ifndef PLATFORM_HAL_H
#define PLATFORM_HAL_H

#ifdef ESP32_PLATFORM
#include "esp32/esp32_hal.h"
#else
#error "No platform selected"
#endif

#endif

/*
Developed by Brandon | Github; A31A18B25C9D012
For public use and modification, see LICENSE file in the root of this repository.
*/