#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <esp_err.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>

#include <lwip/sockets.h>
#include <lwip/dns.h>
#include <lwip/netdb.h>

#include <esp_log.h>
#include <mqtt_client.h>

#include "config.h"
#include "network.h"

#define TAG  "mqtt_example"

static esp_mqtt_client_config_t mqtt_cfg = {
  .broker.address.uri = MQTT_BROKER_URL,
};

static esp_err_t a_mqtt_app_start(void);

static void a_mqtt_on_connect(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void a_mqtt_on_disconnect(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void a_mqtt_on_subscribe(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void a_mqtt_on_unsubscribe(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void a_mqtt_on_publish(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void a_mqtt_on_data(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void a_mqtt_on_error(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

void a_mqtt_main(void) {
  ESP_LOGI(TAG, "[APP] Startup..");
  ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
  ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
  esp_log_level_set("mqtt_example", ESP_LOG_VERBOSE);
  esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
  esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
  esp_log_level_set("transport", ESP_LOG_VERBOSE);
  esp_log_level_set("outbox", ESP_LOG_VERBOSE);

  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  ESP_ERROR_CHECK(a_network_connect());

  a_mqtt_app_start();
}

static esp_err_t a_mqtt_app_start(void) {
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  if (client == NULL) {
    return ESP_FAIL;
  }
  esp_mqtt_client_register_event(client, MQTT_EVENT_CONNECTED,    a_mqtt_on_connect,     NULL);
  esp_mqtt_client_register_event(client, MQTT_EVENT_DISCONNECTED, a_mqtt_on_disconnect,  NULL);
  esp_mqtt_client_register_event(client, MQTT_EVENT_SUBSCRIBED,   a_mqtt_on_subscribe,   NULL);
  esp_mqtt_client_register_event(client, MQTT_EVENT_UNSUBSCRIBED, a_mqtt_on_unsubscribe, NULL);
  esp_mqtt_client_register_event(client, MQTT_EVENT_PUBLISHED,    a_mqtt_on_publish,     NULL);
  esp_mqtt_client_register_event(client, MQTT_EVENT_DATA,         a_mqtt_on_data,        NULL);
  esp_mqtt_client_register_event(client, MQTT_EVENT_ERROR,        a_mqtt_on_error,       NULL);
  esp_mqtt_client_start(client);

  return ESP_OK;
}

static void log_error_if_nonzero(const char *message, int error_code) {
  if (error_code != 0) {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
  }
}

static void a_mqtt_on_connect(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;

  ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
  msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
  ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

  msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
  ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

  msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
  ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

  msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
  ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
};

static void a_mqtt_on_disconnect(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;

  ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
};

static void a_mqtt_on_subscribe(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;

  ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
  msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
  ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
};

static void a_mqtt_on_unsubscribe(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;

  ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
  };

  static void a_mqtt_on_publish(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;

  ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
};

static void a_mqtt_on_data(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;

  ESP_LOGI(TAG, "MQTT_EVENT_DATA");
  printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
  printf("DATA=%.*s\r\n", event->data_len, event->data);
};

static void a_mqtt_on_error(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;

  ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
  if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
    log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
    log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
    log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
    ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
  }
};
