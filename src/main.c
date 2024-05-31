#include "ads1115.h"
#include "bus.h"
#include "config.h"
#include "dht20.h"
#include "mqtt.h"
#include "pico/binary_info.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "util.h"
#include <stdbool.h>
#include <stdio.h>

enum { READ_INTERVAL_MS = 5000 };

void blink_led() {
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
  sleep_ms(250);
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
}

bool init_wifi() {
  printf("Initializing WiFi ...\n");

  if (cyw43_arch_init()) {
    printf("WiFi init failed!\n");
    return false;
  }
  cyw43_arch_enable_sta_mode();

  printf("Connecting to WiFi with SSID \"%s\" ...\n", WIFI_SSID);

  // try 3 times to connect, as sometimes the attempts fail
  for (int i = 0; i < 3; i++) {
    int res = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);

    if (!res) {
      printf("Connected.\n");
      return true;
    }

    if (i < 2) {
      printf(
          "Failed to connect (error: %i), trying again (attempt %i / 3) ...\n",
          res, i + 1);
    }
  }

  printf("Failed to connect.\n");
  return false;
}

void read_and_send_air_data(MQTT_CLIENT_T *mqtt_client) {
  printf("\nReading air data ...\n");

  blink_led();
  dht20_data air_data = dht_read_with_crc();
  mqtt_send_float(mqtt_client, "garden/sensors/temperature0",
                  air_data.temperature);
  mqtt_send_float(mqtt_client, "garden/sensors/humidity0", air_data.humidity);
}

void read_and_send_soil_data(MQTT_CLIENT_T *mqtt_client,
                             uint16_t ads1115_config) {
  printf("\nReading soil data ...\n");

  blink_led();
  sensor_data soil_data =
      ads1115_read_all_values(ADS1115_I2C_ADDR_0, ads1115_config);

  mqtt_send_float(mqtt_client, "garden/sensors/soil0",
                  convert_to_soil_wetness(soil_data.sensor0));
  mqtt_send_float(mqtt_client, "garden/sensors/soil1",
                  convert_to_soil_wetness(soil_data.sensor1));
  mqtt_send_float(mqtt_client, "garden/sensors/soil2",
                  convert_to_soil_wetness(soil_data.sensor2));
  mqtt_send_float(mqtt_client, "garden/sensors/soil3",
                  convert_to_soil_wetness(soil_data.sensor3));

  sensor_data soil_data2 =
      ads1115_read_all_values(ADS1115_I2C_ADDR_1, ads1115_config);

  mqtt_send_float(mqtt_client, "garden/sensors/soil4",
                  convert_to_soil_wetness(soil_data2.sensor0));
  mqtt_send_float(mqtt_client, "garden/sensors/soil5",
                  convert_to_soil_wetness(soil_data2.sensor1));
  mqtt_send_float(mqtt_client, "garden/sensors/soil6",
                  convert_to_soil_wetness(soil_data2.sensor2));
  mqtt_send_float(mqtt_client, "garden/sensors/soil7",
                  convert_to_soil_wetness(soil_data2.sensor3));
}

uint16_t make_ads1115_config() {
  uint16_t config = 0;
  config |= ADS1115_OPT_AIN_0 | ADS1115_OPT_MODE_SINGLE | ADS1115_OPT_RATE_860 |
            ADS1115_OPT_COMP_MODE_TRAD | ADS1115_OPT_ALERT_LO |
            ADS1115_OPT_ALERT_LATCH_OFF | ADS1115_OPT_ALERT_OFF |
            ADS1115_OPT_GAIN_4;
  return config;
}

bool init_network(MQTT_CLIENT_T *mqtt_client) {
  if (!init_wifi())
    return false;

  mqtt_connect(mqtt_client);

  return true;
}

void check_network_status_and_reconnect(MQTT_CLIENT_T *mqtt_client) {
  int wifi_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
  if (wifi_status < 0) {
    printf("Network Error: %i, reconnecting ...\n", wifi_status);
    cyw43_arch_deinit();

    init_network(mqtt_client);
  }
}

int main() {
  stdio_init_all();

  if (!bus_init()) {
    return EXIT_FAILURE;
  }
  scan_bus();
  bool is_dht_available = dht_init();

  uint16_t ads1115_config = make_ads1115_config();
  ads1115_init(ADS1115_I2C_ADDR_0, ads1115_config);
  ads1115_init(ADS1115_I2C_ADDR_1, ads1115_config);

  MQTT_CLIENT_T *mqtt_client = mqtt_client_init();

  if (!init_network(mqtt_client)) {
    return EXIT_FAILURE;
  }

  absolute_time_t next_read = make_timeout_time_ms(READ_INTERVAL_MS);
  bool should_read_soil = false;

  for (;;) {
    cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));

    if (absolute_time_diff_us(get_absolute_time(), next_read) < 0) {
      blink_led();
      next_read = make_timeout_time_ms(READ_INTERVAL_MS);

      if (should_read_soil) {
        read_and_send_soil_data(mqtt_client, ads1115_config);
      } else if (is_dht_available) {
        read_and_send_air_data(mqtt_client);
      }

      check_network_status_and_reconnect(mqtt_client);

      should_read_soil = !should_read_soil;
    }
  }

  cyw43_arch_deinit();
  return EXIT_SUCCESS;
}
