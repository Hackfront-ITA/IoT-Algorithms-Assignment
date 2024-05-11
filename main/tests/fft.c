#include <math.h>
#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/adc.h>
#include <esp_dsp.h>
#include <esp_err.h>
#include <esp_log.h>

#include "tasks/tasks.h"
#include "fft.h"
#include "utils.h"

#define A_ADC_CHANNEL  ADC1_CHANNEL_4
#define A_ADC_ATTEN    ADC_ATTEN_DB_12
#define A_ADC_WIDTH    ADC_WIDTH_BIT_DEFAULT
#define A_ADC_MAX_VAL  4096

#define A_ADAPT_NUM_SAMPLES      2048
#define A_ADAPT_SAMPLING_PERIOD  10
#define A_ADAPT_TOLERANCE        1.03

#define A_DURATION        5000
#define A_FFT_THRESHOLD   25

static const char *TAG = "App main";

static uint16_t calc_sampling_period(void);

void app_main(void) {
	printf("App start\n");

	ESP_ERROR_CHECK(adc1_config_width(A_ADC_WIDTH));
	ESP_ERROR_CHECK(adc1_config_channel_atten(A_ADC_CHANNEL, A_ADC_ATTEN));

	ESP_ERROR_CHECK(a_fft_init());
	ESP_ERROR_CHECK(a_fft_set_size(A_ADAPT_NUM_SAMPLES));

	uint16_t sampling_period = calc_sampling_period();
	if (sampling_period == 65535) {
		ESP_LOGE(TAG, "Error executing adaptation");
		return;
	}

	size_t num_samples = A_DURATION / sampling_period;
	ESP_LOGI(TAG, "new num_samples = %d", num_samples);

	return;
}

static uint16_t calc_sampling_period(void) {
	float *adc_data = malloc(A_ADAPT_NUM_SAMPLES * sizeof(float));
	if (adc_data == NULL) {
		ESP_LOGE(TAG, "Error allocating adc_data (before adapt)");
		return 65535;
	}

	float *fft_data = malloc(A_ADAPT_NUM_SAMPLES * sizeof(float) / 2);
	if (fft_data == NULL) {
		ESP_LOGE(TAG, "Error allocating fft_data");
		return 65535;
	}

	for (size_t i = 0; i < A_ADAPT_NUM_SAMPLES; i++) {
		uint32_t raw_value = adc1_get_raw(A_ADC_CHANNEL);
		// ESP_LOGI(TAG, "adc1 raw value: %ld", raw_value);

		float adc_value = raw_value * 100;
		adc_value /= A_ADC_MAX_VAL;
		adc_data[i] = adc_value;

		vTaskDelay(C_DELAY_MS(A_ADAPT_SAMPLING_PERIOD));
	}

	ESP_LOGW(TAG, "*** View ADC data ***");
	dsps_view(adc_data, 128, 128, 8, -4, +4, '@');

	ESP_ERROR_CHECK(a_fft_execute(adc_data, fft_data));

	ESP_LOGW(TAG, "*** View FFT output ***");
	dsps_view(fft_data, A_ADAPT_NUM_SAMPLES / 2, 128, 10, -60, 40, '*');

	float factor = calc_ps_factor(fft_data, A_ADAPT_NUM_SAMPLES / 2, A_FFT_THRESHOLD);
	ESP_LOGI(TAG, "factor = %f", factor);
	if (factor < 0.001) {
		return 65535;
	}

	uint16_t sampling_period = floor(A_ADAPT_SAMPLING_PERIOD / (A_ADAPT_TOLERANCE * factor));
	float max_freq = 1000.0 / (2.0 * sampling_period);

	ESP_LOGI(TAG, "new sampling_period = %hu", sampling_period);
	ESP_LOGI(TAG, "new max frequency = %f", max_freq);

	free(adc_data);
	free(fft_data);

	return sampling_period;
}
