#include <stddef.h>
#include <stdint.h>

#include <driver/adc.h>
#include <esp_dsp.h>
#include <esp_log.h>

#include "tasks.h"
#include "utils.h"

#define A_ADC_CHANNEL  ADC1_CHANNEL_4
#define A_ADC_ATTEN    ADC_ATTEN_DB_12
#define A_ADC_WIDTH    ADC_WIDTH_BIT_DEFAULT
#define A_ADC_MAX_VAL  4096

static const char *TAG = "Task ADC read";

void task_adc(task_args_t *task_args) {
	uint8_t active_slot = 0;

	while (1) {
		size_t num_samples = task_args->num_samples;
		float *adc_data = task_args->adc_data;
		uint32_t sampling_period = task_args->sampling_period;

		float *cur_data = &adc_data[active_slot * num_samples];

		for (size_t i = 0; i < num_samples; i++) {
			uint32_t raw_value = adc1_get_raw(A_ADC_CHANNEL);
			// ESP_LOGI(TAG, "adc1 raw value: %ld\n", raw_value);

			float adc_value = raw_value / A_ADC_MAX_VAL;
			cur_data[i] = adc_value;

			vTaskDelay(C_DELAY_MS(sampling_period));
		}

		ESP_LOGW(TAG, "*** View ADC data ***");
		dsps_view(cur_data, 128, 128, 10, 0, +1, '@');

		active_slot = (active_slot + 1) % 2;

		vTaskResume(th_dp);
	}
}
