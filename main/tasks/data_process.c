#include <stddef.h>
#include <stdint.h>

#include <esp_log.h>

#include "tasks.h"

static const char *TAG = "Task data process";

void task_data_process(task_args_t *task_args) {
	uint8_t active_slot = 0;

	vTaskSuspend(NULL);

	while (1) {
		ESP_LOGV(TAG, "Task data_process resumed");

		size_t num_samples = task_args->num_samples;
		float *adc_data = task_args->adc_data;

		float *cur_data = &adc_data[active_slot * num_samples];

		float average = 0;
		for (size_t i = 0; i < num_samples; i++) {
			average += cur_data[i];
		}
		average /= num_samples;

		// Transmit via MQTT

		active_slot = (active_slot + 1) % 2;

		vTaskSuspend(NULL);
	}
}
