#include <string.h>

#include <esp_log.h>
#include <esp_netif.h>
#include <esp_wifi.h>

#include "config.h"

#define NET_WIFI_SCAN_METHOD WIFI_FAST_SCAN
// #define NET_WIFI_SCAN_METHOD WIFI_ALL_CHANNEL_SCAN

#define NET_WIFI_CONNECT_AP_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL
// #define NET_WIFI_CONNECT_AP_SORT_METHOD WIFI_CONNECT_AP_BY_SECURITY

// #define NET_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
// #define NET_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
// #define NET_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
// #define NET_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#define NET_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
// #define NET_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_ENTERPRISE
// #define NET_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
// #define NET_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
// #define NET_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK

#define NET_WIFI_CONN_MAX_RETRY  5
#define NET_WIFI_IF_NAME  "netif_wifi"

wifi_config_t wifi_config = {
	.sta = {
		.ssid = NET_WIFI_SSID,
		.password = NET_WIFI_PASSWORD,
		.scan_method = NET_WIFI_SCAN_METHOD,
		.sort_method = NET_WIFI_CONNECT_AP_SORT_METHOD,
		.threshold.rssi = -127,
		.threshold.authmode = NET_WIFI_SCAN_AUTH_MODE_THRESHOLD,
	},
};

static const char *TAG = "hfex_connect";
static esp_netif_t *wifi_interface = NULL;
static int conn_retry_num = 0;

static void wifi_shutdown(void);

static void wifi_handle_disconnect(void *arg,
		esp_event_base_t event_base, int32_t event_id, void *event_data);
static void wifi_handle_connect(void *esp_netif,
		esp_event_base_t event_base, int32_t event_id, void *event_data);
static void wifi_handle_got_ip(void *arg,
		esp_event_base_t event_base, int32_t event_id, void *event_data);

esp_err_t a_network_connect(void) {
	esp_err_t ret;

	ESP_LOGI(TAG, "Start hfex_connect.");

	ESP_ERROR_CHECK(esp_netif_init());

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
	esp_netif_config.if_desc = NET_WIFI_IF_NAME;
	esp_netif_config.route_prio = 128;
	wifi_interface = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
	esp_wifi_set_default_wifi_sta_handlers();

	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());

	conn_retry_num = 0;
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &wifi_handle_connect, wifi_interface));
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_handle_disconnect, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_handle_got_ip, NULL));

	ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

	ret = esp_wifi_connect();
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "WiFi connect failed! ret:%x", ret);
		return ret;
	}

	ESP_ERROR_CHECK(esp_register_shutdown_handler(&wifi_shutdown));

	return ESP_OK;
}

esp_err_t a_network_disconnect(void) {
	wifi_shutdown();
	ESP_ERROR_CHECK(esp_unregister_shutdown_handler(&wifi_shutdown));
	return ESP_OK;
}

static void wifi_shutdown(void) {
	ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &wifi_handle_connect));
	ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_handle_disconnect));
	ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_handle_got_ip));

	esp_wifi_disconnect();

	esp_err_t err = esp_wifi_stop();
	if (err == ESP_ERR_WIFI_NOT_INIT) {
		return;
	}

	ESP_ERROR_CHECK(err);
	ESP_ERROR_CHECK(esp_wifi_deinit());
	ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(wifi_interface));
	esp_netif_destroy(wifi_interface);
	wifi_interface = NULL;
}

static void wifi_handle_disconnect(void *arg,
		esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	conn_retry_num++;
	if (conn_retry_num > NET_WIFI_CONN_MAX_RETRY) {
		ESP_LOGI(TAG, "WiFi Connect failed %d times, stop reconnect.", conn_retry_num);
		return;
	}

	ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
	esp_err_t err = esp_wifi_connect();
	if (err == ESP_ERR_WIFI_NOT_STARTED) {
		return;
	}

	ESP_ERROR_CHECK(err);
}

static void wifi_handle_connect(void *esp_netif,
		esp_event_base_t event_base, int32_t event_id, void *event_data)
{
}

static void wifi_handle_got_ip(void *arg,
		esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	conn_retry_num = 0;
	ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
	if (strncmp(NET_WIFI_IF_NAME, esp_netif_get_desc(event->esp_netif), strlen(NET_WIFI_IF_NAME) - 1) != 0) {
		return;
	}
	ESP_LOGI(TAG, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
	ESP_LOGI(TAG, "- IPv4 address: " IPSTR ",", IP2STR(&event->ip_info.ip));
}
