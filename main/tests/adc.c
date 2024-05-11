#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/adc.h>
#include <esp_err.h>
#include <esp_log.h>

#include "utils.h"

#define A_ADC_CHANNEL  ADC1_CHANNEL_4
#define A_ADC_ATTEN    ADC_ATTEN_DB_12
#define A_ADC_WIDTH    ADC_WIDTH_BIT_DEFAULT
#define A_ADC_MAX_VAL  4096

#define A_ADAPT_SAMPLING_PERIOD  100

static const char *TAG = "App main";

void app_main(void) {
	printf("App start\n");

	ESP_ERROR_CHECK(adc1_config_width(A_ADC_WIDTH));
	ESP_ERROR_CHECK(adc1_config_channel_atten(A_ADC_CHANNEL, A_ADC_ATTEN));

	while (1) {
		uint32_t raw_value = adc1_get_raw(A_ADC_CHANNEL);

		float calc_value = raw_value;
		calc_value /= A_ADC_MAX_VAL;

		ESP_LOGI(TAG, "raw_value = %05ld, calc_value %f", raw_value, calc_value);

		vTaskDelay(C_DELAY_MS(A_ADAPT_SAMPLING_PERIOD));
	}

	return;
}
