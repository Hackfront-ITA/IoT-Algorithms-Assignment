#include <math.h>
#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/adc.h>
#include <esp_err.h>
#include <esp_log.h>

#include "utils.h"

#define A_ADC_CHANNEL        ADC1_CHANNEL_4
#define A_ADC_ATTEN          ADC_ATTEN_DB_12
#define A_ADC_BIT_WIDTH      ADC_WIDTH_BIT_DEFAULT

#define A_ADC_MAX_VAL        4096

esp_err_t a_adc_init(void) {
  esp_err_t err;

	err = adc1_config_width(A_ADC_BIT_WIDTH);
	if (err != ESP_OK) {
		return err;
	}

	err = adc1_config_channel_atten(A_ADC_CHANNEL, A_ADC_ATTEN);
	if (err != ESP_OK) {
		return err;
	}

  return ESP_OK;
}

esp_err_t a_adc_collect_samples(float *buffer, size_t length, float sampling_freq) {
	uint16_t sampling_period = floor(1000.0 / sampling_freq);

	for (size_t i = 0; i < length; i++) {
		uint32_t raw_value = adc1_get_raw(A_ADC_CHANNEL);
		// ESP_LOGI(TAG, "adc1 raw value: %ld\n", raw_value);

		float adc_value = raw_value;
		raw_value *= 100.0;
		raw_value /= A_ADC_MAX_VAL;
		buffer[i] = adc_value;

		vTaskDelay(C_DELAY_MS(sampling_period));
	}

  return ESP_OK;
}
