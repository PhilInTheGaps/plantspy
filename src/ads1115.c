#include "ads1115.h"
#include "bus.h"
#include <stdio.h>

#define ADS1115_CONVERSION_REG 0x0
#define ADS1115_CONFIG_REG 0x1
#define ADS1115_LO_THRESH_REG 0x2
#define ADS1115_HI_THRESH_REG 0x3

void ads1115_init(uint8_t addr, uint16_t config) {
  const uint8_t lsb = config & 0xFF;
  const uint8_t msb = config >> 8;

  const uint8_t com[3] = {ADS1115_CONFIG_REG, msb, lsb};
  i2c_write_blocking(I2C_PORT, addr, com, 3, false);
}

void ads1115_change_input(uint8_t addr, uint16_t config, uint16_t input) {
  const uint16_t mask = 0x8FFF;
  config &= mask;
  config |= input;

  ads1115_init(addr, config);
}

int16_t ads1115_read_value(uint8_t addr) {
  // set read register to config register
  uint8_t reg = ADS1115_CONFIG_REG;
  i2c_write_blocking(I2C_PORT, addr, &reg, 1, false);

  uint8_t response[2];

  for (;;) {
    // read to check if measurement has finished
    int res = i2c_read_blocking(I2C_PORT, addr, response, 2, false);

    if (res == PICO_ERROR_GENERIC) {
      printf("Could not read ADS1115 data at I2C address %#x\n", addr);
      return 0;
    }

    // if ready bit is set, continue
    if (response[0] & (1 << 7)) {
      break;
    }

    sleep_ms(5);
  }

  // set read register to conversion register
  i2c_write_blocking(I2C_PORT, addr, ADS1115_CONVERSION_REG, 1, false);
  i2c_read_blocking(I2C_PORT, addr, response, 2, false);

  return (int16_t)(((uint16_t)response[0] << 8) | response[1]);
}

sensor_data ads1115_read_all_values(uint8_t addr, uint16_t config) {
  config |= ADS1115_OPT_BEGIN_SINGLE; // start a measurement

  sensor_data data;

  ads1115_change_input(addr, config, ADS1115_OPT_AIN_0);
  data.sensor0 = ads1115_read_value(addr);

  ads1115_change_input(addr, config, ADS1115_OPT_AIN_1);
  data.sensor1 = ads1115_read_value(addr);

  ads1115_change_input(addr, config, ADS1115_OPT_AIN_2);
  data.sensor2 = ads1115_read_value(addr);

  ads1115_change_input(addr, config, ADS1115_OPT_AIN_3);
  data.sensor3 = ads1115_read_value(addr);

  return data;
}
