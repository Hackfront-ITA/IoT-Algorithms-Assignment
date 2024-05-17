#include <stdint.h>

#include <esp_log.h>
#include <mqtt_client.h>

#include "config.h"

static const char *TAG = "MQTT";

extern const uint8_t a_mqtt_cert[]   asm("_binary_mqtt_cert_pem_start");

static esp_mqtt_client_config_t a_mqtt_cfg = {
  .broker = {
    .address.uri = MQTT_BROKER_URL,
    .verification.certificate = (const char *)(a_mqtt_cert)
  }
};

static esp_mqtt_client_handle_t a_client;

static void a_mqtt_on_connect(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void a_mqtt_on_disconnect(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void a_mqtt_on_error(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

esp_err_t a_mqtt_start(void) {
  a_client = esp_mqtt_client_init(&a_mqtt_cfg);
  if (a_client == NULL) {
    return ESP_FAIL;
  }
  esp_mqtt_client_register_event(a_client, MQTT_EVENT_CONNECTED,    a_mqtt_on_connect,     NULL);
  esp_mqtt_client_register_event(a_client, MQTT_EVENT_DISCONNECTED, a_mqtt_on_disconnect,  NULL);
  esp_mqtt_client_register_event(a_client, MQTT_EVENT_ERROR,        a_mqtt_on_error,       NULL);
  esp_mqtt_client_start(a_client);

  return ESP_OK;
}

esp_err_t a_mqtt_stop(void) {
  return ESP_OK;
}

esp_err_t a_mqtt_publish(const char *topic, const char *data) {
  int msg_id;

  msg_id = esp_mqtt_client_enqueue(a_client, topic, data, 0, 0, 0, true);

  if (msg_id < 0) {
    ESP_LOGE(TAG, "Error publishing message, rc=%d", msg_id);
    return ESP_FAIL;
  }

  return ESP_OK;
}


static void a_mqtt_on_connect(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "Connected");

  int msg_id = esp_mqtt_client_publish(a_client, "/tests/esp32/status", "ALIVE", 0, 0, true);
  ESP_LOGI(TAG, "Sent alive message, msg_id=%d", msg_id);
};

static void a_mqtt_on_disconnect(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "Disconnected");
};

static void a_mqtt_on_error(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "Generic error");

  // if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
  //   ESP_LOGE(TAG, "Last error reported from esp-tls: 0x%x",
  //       event->error_handle->esp_tls_last_esp_err);
  //
  //   ESP_LOGE(TAG, "Last error reported from tls stack: 0x%x",
  //       event->error_handle->esp_tls_stack_err);
  //
  //   ESP_LOGE(TAG, "Last error captured as transport's socket errno: 0x%x",
  //       event->error_handle->esp_transport_sock_errno);
  //
  //   ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
  // }
};
