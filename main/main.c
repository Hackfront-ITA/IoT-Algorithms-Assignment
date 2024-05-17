#include <math.h>
#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_dsp.h>
#include <esp_err.h>
#include <esp_log.h>

#include "adapt.h"
#include "adc.h"
#include "fft.h"
#include "utils.h"

#define A_DURATION  5000

static const char *TAG = "App main";

void app_main(void) {
	ESP_LOGI(TAG, "App start");

	ESP_LOGI(TAG, "Init ADC");
	ESP_ERROR_CHECK(a_adc_init());

	ESP_LOGI(TAG, "Sense sampling frequency");
	float sampling_freq = sense_sampling_freq();
	if (sampling_freq < 0.001) {
		ESP_LOGE(TAG, "Error executing sample rate adaptation");
		return;
	}

	sampling_freq = 1000;

	float max_freq = sampling_freq / 2.0;
	float flush_freq = sampling_freq * 1.2;
	size_t num_samples = A_DURATION * sampling_freq / 1000.0;

	ESP_LOGI(TAG, "new max frequency = %f", max_freq);
	ESP_LOGI(TAG, "ADC buffer flush frequency = %f", flush_freq);
	ESP_LOGI(TAG, "new num_samples = %d", num_samples);

	ESP_ERROR_CHECK(a_adc_set_sampling_freq(sampling_freq));

	ESP_LOGI(TAG, "calloc(%d, %d)", 4 * num_samples, sizeof(float));
	float *adc_data = calloc(4 * num_samples, sizeof(float));
	if (adc_data == NULL) {
		ESP_LOGE(TAG, "Error allocating adc_data (after adapt)");
		return;
	}

	ESP_LOGI(TAG, "Connect to network");
	// a_network_connect();

	ESP_LOGI(TAG, "Start sampling");
	ESP_ERROR_CHECK(a_adc_start());

	ESP_LOGI(TAG, "Running...");
	while (1) {
		ESP_ERROR_CHECK(a_adc_collect_samples(adc_data, num_samples, flush_freq));

		ESP_LOGI(TAG, "*** View ADC data ***");
		if (esp_log_level_get(TAG) == ESP_LOG_INFO) {
			dsps_view(adc_data, num_samples, 128, 10, 0, +100, '@');
		}

		float average = calc_average(adc_data, num_samples);

		ESP_LOGI(TAG, "average = %f", average);

		// Transmit via MQTT
	}

	ESP_LOGI(TAG, "Stop sampling");
	ESP_ERROR_CHECK(a_adc_stop());

	free(adc_data);

	return;
}
