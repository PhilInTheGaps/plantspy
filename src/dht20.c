#include "dht20.h"
#include "bus.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define DHT20_ADDR _u(0x38)

uint8_t dht_calc_crc(const uint8_t *data, uint8_t len) {
  uint8_t i = 0;
  uint8_t crc = 0xFF;

  for (uint8_t byte = 0; byte < len; byte++) {
    crc ^= (data[byte]);
    for (i = 8; i > 0; --i) {
      if (crc & 0x80)
        crc = (crc << 1) ^ 0x31;
      else
        crc = (crc << 1);
    }
  }

  return crc;
}

uint8_t dht_get_status() {
  uint8_t status = 0;
  i2c_read_blocking(I2C_PORT, DHT20_ADDR, &status, 1, false);

  return status;
}

bool dht_init() {
  sleep_ms(100);
  uint8_t status = dht_get_status();

  if (status != 0x18) {
    printf("Error: DHT20 status is not OK: 0x%X\n", status);
    return false;
  }

  printf("DHT20 has been correctly initialized!\n");

  if ((status & 0x80) == 0x80) {
    // init
    uint8_t com[3] = {0xa8, 0x00, 0x00};
    i2c_write_blocking(I2C_PORT, DHT20_ADDR, com, 3, false);

    sleep_ms(10);

    uint8_t com2[3] = {0xbe, 0x08, 0x00};
    i2c_write_blocking(I2C_PORT, DHT20_ADDR, com2, 3, false);
  }

  return true;
}

uint8_t dht_wait_for_measurement() {
  sleep_ms(10);
  uint8_t com[3] = {0xac, 0x33, 0x00};
  i2c_write_blocking(I2C_PORT, DHT20_ADDR, com, 3, false);

  uint8_t count = 0;
  while ((dht_get_status() & 0x80) == 0x80) {
    sleep_ms(1);
    if (count >= 100)
      break;
    count++;
  }

  return count;
}

double dht_calculate_humidity(const uint8_t *data, uint8_t len) {
  if (len < 6)
    return 0;

  uint32_t value = 0;
  value = (value | data[1]) << 8;
  value = (value | data[2]) << 8;
  value = (value | data[3]);
  value = value >> 4;

  double humidity = (value * 100 * 10 / 1024.0 / 1024.0) / 10.0;
  return humidity;
}

double dht_calculate_temperature(const uint8_t *data, uint8_t len) {
  if (len < 6)
    return 0;

  uint32_t value = 0;
  value = (value | data[3]) << 8;
  value = (value | data[4]) << 8;
  value = (value | data[5]);
  value = value & 0xfffff;

  double temperature = (value * 200 * 10 / 1024.0 / 1024.0 - 500) / 10.0;
  return temperature;
}

dht20_data dht_read_with_crc() {
  dht_wait_for_measurement();

  // read
  uint8_t buf[7];
  i2c_read_blocking(I2C_PORT, DHT20_ADDR, buf, 7, false);

  //    printf("Measurement Data:\n");
  //    for (int i = 0; i < 7; i++) {
  //        printf("0x%x\n", buf[i]);
  //    }

  uint8_t crc = dht_calc_crc(buf, 6);
  //    printf("CRC: 0x%x\n", crc);

  dht20_data data = {0, 0};

  if (crc == buf[6]) {
    data.humidity = dht_calculate_humidity(buf, 7);
    data.temperature = dht_calculate_temperature(buf, 7);
  } else {
    printf("Could not read measurements, CRC does not match!\n");
  }

  return data;
}
