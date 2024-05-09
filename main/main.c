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

#define A_ADAPT_NUM_SAMPLES      4096
#define A_ADAPT_SAMPLING_PERIOD  1000
#define A_DURATION  5000
#define A_FFT_THRESHOLD   0

#define A_TASK_STACK_SIZE  1024

static task_args_t task_args;

static const char *TAG = "App main";

TaskHandle_t th_adc = NULL;
TaskHandle_t th_dp = NULL;

static uint16_t calc_sampling_period(void);

void app_main(void) {
	printf("App start\n");

	ESP_ERROR_CHECK(adc1_config_width(A_ADC_WIDTH));
	ESP_ERROR_CHECK(adc1_config_channel_atten(A_ADC_CHANNEL, A_ADC_ATTEN));

	ESP_ERROR_CHECK(a_fft_init());
	ESP_ERROR_CHECK(a_fft_set_size(A_ADAPT_NUM_SAMPLES));

	uint16_t sampling_period = calc_sampling_period();
	if (sampling_period == 0) {
		ESP_LOGE(TAG, "Error executing adaptation\n");
		return;
	}

	size_t num_samples = A_DURATION / sampling_period;

	float *adc_data = malloc(2 * num_samples * sizeof(float));
	if (adc_data == NULL) {
		ESP_LOGE(TAG, "Error allocating adc_data (after adapt)\n");
		return;
	}

	task_args.sampling_period = sampling_period;
	task_args.num_samples = num_samples;
	task_args.adc_data = adc_data;

	ESP_LOGI(TAG, "Create ADC read task\n");
	xTaskCreate(task_adc,          "ADC read task",        A_TASK_STACK_SIZE, &task_args, 10, &th_adc);
	ESP_LOGI(TAG, "Create data processing task\n");
	xTaskCreate(task_data_process, "Data processing task", A_TASK_STACK_SIZE, &task_args, 10, &th_dp);

	return;
}

static uint16_t calc_sampling_period(void) {
	float *adc_data = malloc(A_ADAPT_NUM_SAMPLES * sizeof(float));
	if (adc_data == NULL) {
		ESP_LOGE(TAG, "Error allocating adc_data (before adapt)");
		return 0;
	}

	float *fft_data = malloc(A_ADAPT_NUM_SAMPLES * sizeof(float));
	if (fft_data == NULL) {
		ESP_LOGE(TAG, "Error allocating fft_data");
		return 0;
	}

	for (size_t i = 0; i < A_ADAPT_NUM_SAMPLES; i++) {
		uint32_t raw_value = adc1_get_raw(A_ADC_CHANNEL);
		ESP_LOGI(TAG, "adc1 raw value: %ld\n", raw_value);

		float adc_value = raw_value / A_ADC_MAX_VAL;
		adc_data[i] = adc_value;

		vTaskDelay(C_DELAY_MS(A_ADAPT_SAMPLING_PERIOD));
	}

	ESP_LOGW(TAG, "*** View ADC data ***");
	dsps_view(adc_data, 128, 128, 10, 0, +1, '@');

	ESP_ERROR_CHECK(a_fft_execute(adc_data, fft_data));

	ESP_LOGW(TAG, "*** View FFT output ***");
	dsps_view(fft_data, A_ADAPT_NUM_SAMPLES / 2, 128, 10, -60, 40, '*');

	float factor = calc_ps_factor(fft_data, A_ADAPT_NUM_SAMPLES / 2, A_FFT_THRESHOLD);
	ESP_LOGI(TAG, "factor = %f\n", factor);

	uint16_t sampling_period = A_ADAPT_SAMPLING_PERIOD / factor;
	float max_freq = 1 / (2 * sampling_period);

	ESP_LOGI(TAG, "new sampling_period = %hd\n", sampling_period);
	ESP_LOGI(TAG, "new max frequency = %f\n", max_freq);

	free(adc_data);
	free(fft_data);

	return sampling_period;
}
