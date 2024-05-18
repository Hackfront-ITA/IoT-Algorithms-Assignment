#include <stddef.h>
#include <stdint.h>

#include <driver/adc.h>
#include <esp_dsp.h>
#include <esp_log.h>

#include "tasks/tasks.h"
#include "adc.h"
#include "utils.h"

static const char *TAG = "Task data read";

void task_data_read(task_args_t *task_args) {
	uint8_t active_slot = 0;

	size_t num_samples = task_args->num_samples;
	float *adc_data = task_args->adc_data;
	float sampling_freq = task_args->sampling_freq;

	while (1) {
		float *cur_data = &adc_data[active_slot * num_samples];

		a_adc_collect_samples(cur_data, num_samples, sampling_freq);

		active_slot = (active_slot + 1) % 2;

		vTaskResume(th_data_process);
	}
}
