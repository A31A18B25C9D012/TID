A Thermal Imaging Driver framework supporting multiple IR sensors with unified API.

## Supported Sensors

- MLX90640 (32x24 pixels, I2C)
- AMG8833 (8x8 pixels, I2C)

### Features

1. Hardware-agnostic transport layer (I2C, SPI)
2. Automatic calibration loading and temperature conversion
3. Frame processing: min/max detection, hotspot finding, interpolation, median filtering
4. Colormap conversion to RGB565
5. ESP32-S3 platform support with dual-core capability
6. No dynamic memory allocation in frame loop
7. Complete error handling with status codes

### Building

```bash
make clean > Clean build
make > Compile script
make run > Run the framwork
```

### Compiler Requirements

- C11 standard
- -Wall -Wextra clean compilation
- Math library (-lm)

### Calibration

* MLX90640: Automatic EEPROM calibration with gain, offset, ambient temperature compensation
* AMG8833: Built-in offset and gain correction

All temperature conversions use float arithmetic for accuracy.

### Error Handling

All functions return thermal_status_t with explicit error codes:
- `THERMAL_OK`: Success
- `THERMAL_ERR_INVALID_ARG`: Invalid argument
- `THERMAL_ERR_TIMEOUT`: Communication timeout
- `THERMAL_ERR_IO`: I/O error
- `THERMAL_ERR_NOT_INIT`: Device not initialized
- `THERMAL_ERR_CALIBRATION`: Calibration failure
- `THERMAL_ERR_FRAME_INVALID`: Invalid frame data
- `THERMAL_ERR_UNSUPPORTED`: Unsupported operation
- `THERMAL_ERR_BUS`: Bus error
- `THERMAL_ERR_CHECKSUM`: Checksum error
- `THERMAL_ERR_RESET`: Reset required

### Processing Functions

- `thermal_find_minmax()`: Locate minimum and maximum temperature points
- `thermal_find_hotspots()`: Detect local temperature maxima above threshold
- `thermal_interpolate_bilinear()`: Upscale frames using bilinear interpolation
- `thermal_median_filter()`: Apply median filter for noise reduction
- `thermal_apply_colormap()`: Convert temperature data to RGB565 colormap

### Porting to New Platforms

1. Implement platform HAL in platform/your_platform/
2. Create your_platform_hal.h with I2C/SPI function prototypes
3. Implement hardware-specific I2C/SPI functions
4. Update platform_hal.h with platform selection
5. Modify Makefile to include new platform sources

No changes required to core, sensor, or transport layers.

### Adding New Sensors

1. Create sensor header in include/sensors/
2. Implement sensor_ops_t structure
3. Implement calibration extraction
4. Implement temperature conversion
5. Add sensor to build system

Sensor must implement:
- `init()`: Initialize and load calibration
- `get_frame()`: Acquire and convert frame
- `get_resolution()`: Return sensor resolution
- `set_refresh_rate()`: Configure frame rate
- `self_test()`: Verify sensor functionality
- `shutdown()`: Power down sensor

### Performance Characteristics

1. Zero heap allocation in acquisition loop
2. Static frame buffers
3. Configurable fixed-point math path
4. Transport retry with backoff
5. Optimized bilinear interpolation
6. Median filter with bounds checking
