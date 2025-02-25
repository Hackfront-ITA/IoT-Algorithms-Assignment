#include <math.h>
#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_dsp.h>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include "adapt.h"
#include "adc.h"
#include "config.h"
#include "fft.h"
#include "mqtt.h"
#include "network.h"
#include "utils.h"

#define A_DO_ADAPTATION        1
#define A_FIXED_SAMPLING_FREQ  (2 * 1000)

#define A_WINDOW_LEN           1000

static const char *TAG = "App main";

void app_main(void) {
	ESP_LOGI(TAG, "App start");

	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	ESP_LOGI(TAG, "Init ADC");
	ESP_ERROR_CHECK(a_adc_init());

	float sampling_freq;

#if A_DO_ADAPTATION == 1
	ESP_LOGI(TAG, "Sense sampling frequency");
	sampling_freq = sense_sampling_freq();
	if (sampling_freq < 0.001) {
		ESP_LOGE(TAG, "Error executing sample rate adaptation");
		return;
	}
	ESP_LOGW(TAG, "Sensed sampling frequency: %.02f Hz", sampling_freq);
#else
	sampling_freq = A_FIXED_SAMPLING_FREQ;
	ESP_LOGW(TAG, "Fixed sampling frequency: %.02f Hz", sampling_freq);
#endif

	sampling_freq = fmax(sampling_freq, SOC_ADC_SAMPLE_FREQ_THRES_LOW);
	sampling_freq = fmin(sampling_freq, SOC_ADC_SAMPLE_FREQ_THRES_HIGH);
	ESP_LOGI(TAG, "actual ADC sampling frequency: %.02f Hz", sampling_freq);


	float max_freq = sampling_freq / 2.0;
	float flush_freq = sampling_freq * 1.2;
	size_t num_samples = A_WINDOW_LEN * sampling_freq / 1000.0;

	ESP_LOGI(TAG, "new max frequency = %f", max_freq);
	ESP_LOGI(TAG, "ADC buffer flush frequency = %f", flush_freq);
	ESP_LOGI(TAG, "new num_samples = %d", num_samples);

	ESP_ERROR_CHECK(a_adc_set_sampling_freq(sampling_freq));

	ESP_LOGI(TAG, "calloc(%d, %d)", num_samples, sizeof(float));
	float *adc_data = calloc(2 * num_samples, sizeof(float));
	if (adc_data == NULL) {
		ESP_LOGE(TAG, "Error allocating adc_data (after adapt)");
		return;
	}

	ESP_LOGI(TAG, "Connect to network");
	ESP_ERROR_CHECK(a_network_connect());

	ESP_LOGI(TAG, "Connect to MQTT server");
	ESP_ERROR_CHECK(a_mqtt_start());

	ESP_LOGI(TAG, "Start sampling");
	ESP_ERROR_CHECK(a_adc_start());

	ESP_LOGI(TAG, "Running...");
	while (1) {
		ESP_ERROR_CHECK(a_adc_collect_samples(adc_data, num_samples, flush_freq));

		ESP_LOGI(TAG, "*** View ADC data ***");
		if (esp_log_level_get(TAG) == ESP_LOG_INFO) {
			dsps_view(adc_data, num_samples, 128, 10, 0, +100, '@');
		}

		ESP_LOGI(TAG, "Calculating average");
		float average = calc_average(adc_data, num_samples);
		ESP_LOGI(TAG, "average = %f", average);

		if (a_mqtt_connected) {
			ESP_LOGI(TAG, "Sending average");

			char buffer[16];
			snprintf(buffer, sizeof(buffer), "%.05f", average);

			ESP_ERROR_CHECK(a_mqtt_publish(MQTT_BASE_TOPIC "/average", buffer));
		}
	}

	ESP_LOGI(TAG, "Stop sampling");
	ESP_ERROR_CHECK(a_adc_stop());

	ESP_LOGI(TAG, "Disconnect from MQTT server");
	ESP_ERROR_CHECK(a_mqtt_stop());

	ESP_LOGI(TAG, "Disconnect from network");
	ESP_ERROR_CHECK(a_network_disconnect());

	free(adc_data);

	return;
}
