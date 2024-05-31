#pragma once

#include "hardware/i2c.h"
#include <stdbool.h>

// I2C defines
// Uses GP4 and GP5
#define I2C_PORT i2c_default
#define I2C_SDA PICO_DEFAULT_I2C_SDA_PIN
#define I2C_SCL PICO_DEFAULT_I2C_SCL_PIN

bool bus_init();
void scan_bus();
