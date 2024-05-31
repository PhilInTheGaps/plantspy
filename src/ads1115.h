#pragma once

#include "pico/stdlib.h"

// Different I2C addresses configurable by connection ADDR pin
#define ADS1115_I2C_ADDR_0 0x48 // GND
#define ADS1115_I2C_ADDR_1 0x49 // VDD
#define ADS1115_I2C_ADDR_2 0x4A // SDA
#define ADS1115_I2C_ADDR_3 0x4B // SCL

// Options to set in 16 bit config register
#define ADS1115_OPT_BEGIN_SINGLE 0x8000

#define ADS1115_OPT_AIN_0 0x4000
#define ADS1115_OPT_AIN_1 0x5000
#define ADS1115_OPT_AIN_2 0x6000
#define ADS1115_OPT_AIN_3 0x7000

#define ADS1115_OPT_GAIN_6 0x0      // PGA 2/3, FS: 6.144V
#define ADS1115_OPT_GAIN_4 0x200    // PGA 1, FS: 4.096V
#define ADS1115_OPT_GAIN_2 0x400    // PGA 2, FS: 2.048V
#define ADS1115_OPT_GAIN_1 0x600    // PGA 4, FS: 1.024V
#define ADS1115_OPT_GAIN_0_5 0x800  // PGA 8, FS: 0.512V
#define ADS1115_OPT_GAIN_0_25 0xE00 // PGA 16, FS: 0.256V

#define ADS1115_OPT_MODE_CONT 0x0 // default mode
#define ADS1115_OPT_MODE_SINGLE 0x100

#define ADS1115_OPT_RATE_8 0x0
#define ADS1115_OPT_RATE_16 0x20
#define ADS1115_OPT_RATE_32 0x40
#define ADS1115_OPT_RATE_64 0x60
#define ADS1115_OPT_RATE_128 0x80
#define ADS1115_OPT_RATE_250 0xA0
#define ADS1115_OPT_RATE_475 0xC0
#define ADS1115_OPT_RATE_860 0xE0

#define ADS1115_OPT_COMP_MODE_TRAD 0x0
#define ADS1115_OPT_COMP_MODE_WINDOW 0x10

#define ADS1115_OPT_ALERT_LO 0x0
#define ADS1115_OPT_ALERT_HI 0x8

#define ADS1115_OPT_ALERT_LATCH_OFF 0x0
#define ADS1115_OPT_ALERT_LATCH_ON 0x4

#define ADS1115_OPT_ALERT_AFTER_1 0x0
#define ADS1115_OPT_ALERT_AFTER_2 0x1
#define ADS1115_OPT_ALERT_AFTER_3 0x2
#define ADS1115_OPT_ALERT_OFF 0x3

typedef struct sensor_data sensor_data;
struct sensor_data {
  int16_t sensor0;
  int16_t sensor1;
  int16_t sensor2;
  int16_t sensor3;
};

void ads1115_init(uint8_t addr, uint16_t config);
int16_t ads1115_read_value(uint8_t addr);
sensor_data ads1115_read_all_values(uint8_t addr, uint16_t config);
