#include "mqtt.h"
#include "config.h"
#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>

void dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
  MQTT_CLIENT_T *state = (MQTT_CLIENT_T *)callback_arg;
  printf("DNS query for \"%s\" finished with resolved addr of %s.\n", name,
         ip4addr_ntoa(ipaddr));
  state->remote_addr = *ipaddr;
}

void run_dns_lookup_blocking(MQTT_CLIENT_T *state) {
  printf("Running DNS query for \"%s\".\n", MQTT_SERVER_HOST);

  cyw43_arch_lwip_begin();
  err_t err = dns_gethostbyname(MQTT_SERVER_HOST, &(state->remote_addr),
                                dns_found, state);
  cyw43_arch_lwip_end();

  if (err == ERR_ARG) {
    printf("Failed to start DNS query!\n");
    return;
  }

  if (err == ERR_OK) {
    printf("No lookup needed.");
    return;
  }

  while (state->remote_addr.addr == 0) {
    cyw43_arch_poll();
  }
}

MQTT_CLIENT_T *mqtt_client_init(void) {
  MQTT_CLIENT_T *state = calloc(1, sizeof(MQTT_CLIENT_T));
  if (!state) {
    printf("Failed to allocate mqtt state!\n");
    return NULL;
  }
  state->receiving = 0;
  state->received = 0;
  return state;
}

void mqtt_connection_cb(mqtt_client_t *client, void *arg,
                        mqtt_connection_status_t status) {
  if (status != 0) {
    printf("Error during connection: err %d.\n", status);
  } else {
    printf("MQTT connected.\n");
  }
}

void mqtt_pub_request_cb(void *arg, err_t err) {
  MQTT_CLIENT_T *state = (MQTT_CLIENT_T *)arg;

  if (err != ERR_OK) {
    printf("MQTT publishing error: %d\n", err);
  }

  state->receiving = 0;
  state->received++;
}

err_t mqtt_send_msg(MQTT_CLIENT_T *state, const char *topic,
                    const char *message) {
  err_t err = 0;
  u8_t qos = AtMostOnce;
  u8_t retain = 0;

  printf("Sending to \"%s\": %s\n", topic, message);

  if (!mqtt_client_is_connected(state->mqtt_client)) {
    printf("Could not send mqtt message: not connected\n");
    return ERR_CONN;
  }

  do {
    cyw43_arch_lwip_begin();
    err = mqtt_publish(state->mqtt_client, topic, message, strlen(message), qos,
                       retain, mqtt_pub_request_cb, state);
    cyw43_arch_lwip_end();

    if (err == ERR_MEM) {
      cyw43_arch_poll();
    }

  } while (err == ERR_MEM);

  if (err == ERR_CONN) {
    printf("Could not send mqtt message: not connected\n");
  }

  return err;
}

err_t mqtt_send_int(MQTT_CLIENT_T *state, const char *topic, int value) {
  int length = snprintf(NULL, 0, "%d", value);
  char str[length];
  sprintf(str, "%d", value);

  return mqtt_send_msg(state, topic, str);
}

err_t mqtt_send_float(MQTT_CLIENT_T *state, const char *topic, double value) {
  int length = snprintf(NULL, 0, "%.1f", value);
  char str[length];
  sprintf(str, "%.1f", value);

  return mqtt_send_msg(state, topic, str);
}

err_t mqtt_connect(MQTT_CLIENT_T *state) {
  printf("\nConnecting to MQTT server at %s:%i ...\n", MQTT_SERVER_HOST,
         MQTT_SERVER_PORT);
  state->mqtt_client = mqtt_client_new();
  state->counter = 0;

  if (state->mqtt_client == NULL) {
    printf("Failed to create new mqtt client\n");
    return -1;
  }

  run_dns_lookup_blocking(state);

  struct mqtt_connect_client_info_t ci;
  err_t err = 0;

  memset(&ci, 0, sizeof(ci));
  ci.client_id = MQTT_CLIENT_ID;
  ci.client_user = MQTT_USER;
  ci.client_pass = MQTT_PASSWORD;
  ci.keep_alive = 0;
  ci.will_topic = NULL;
  ci.will_msg = NULL;
  ci.will_retain = 0;
  ci.will_qos = 0;

  err = mqtt_client_connect(state->mqtt_client, &(state->remote_addr),
                            MQTT_SERVER_PORT, mqtt_connection_cb, state, &ci);

  if (err != ERR_OK) {
    printf("Could not connect to MQTT server: %d\n", err);
  }

  return err;
}
