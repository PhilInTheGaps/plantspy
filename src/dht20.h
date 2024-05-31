#pragma once

#include <stdbool.h>

typedef struct dht20_data dht20_data;
struct dht20_data {
  double humidity;
  double temperature;
};

bool dht_init();
dht20_data dht_read_with_crc();
