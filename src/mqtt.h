#pragma once

#include "lwip/apps/mqtt.h"

typedef struct MQTT_CLIENT_T MQTT_CLIENT_T;
struct MQTT_CLIENT_T {
  ip_addr_t remote_addr;
  mqtt_client_t *mqtt_client;
  u8_t receiving;
  u32_t received;
  u32_t counter;
  u32_t reconnect;
};

enum MQTT_QOS { AtMostOnce = 0, AtLeastOnce = 1, ExactlyOnce = 2 };

MQTT_CLIENT_T *mqtt_client_init();
err_t mqtt_connect(MQTT_CLIENT_T *state);
err_t mqtt_send_msg(MQTT_CLIENT_T *state, const char *topic,
                    const char *message);
err_t mqtt_send_int(MQTT_CLIENT_T *state, const char *topic, int value);
err_t mqtt_send_float(MQTT_CLIENT_T *state, const char *topic, double value);
