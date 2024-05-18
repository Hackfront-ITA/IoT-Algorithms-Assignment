#include <stddef.h>
#include <stdint.h>

#include <esp_dsp.h>
#include <esp_log.h>

#include "tasks/tasks.h"
#include "utils.h"

static const char *TAG = "Task data process";

void task_data_process(task_args_t *task_args) {
	uint8_t active_slot = 0;

	vTaskSuspend(NULL);

	size_t num_samples = task_args->num_samples;
	float *adc_data = task_args->adc_data;

	while (1) {
		ESP_LOGV(TAG, "Task data_process resumed");

		float *cur_data = &adc_data[active_slot * num_samples];

		ESP_LOGI(TAG, "*** View ADC data ***");
		if (esp_log_level_get(TAG) == ESP_LOG_INFO) {
			dsps_view(cur_data, num_samples, 128, 10, 0, +100, '@');
		}

		float average = calc_average(cur_data, num_samples);
		ESP_LOGI(TAG, "average = %f", average);

		// if (a_mqtt_connected) {
		// 	ESP_LOGI(TAG, "Sending average");
		//
		// 	char buffer[16];
		// 	snprintf(buffer, sizeof(buffer), "%.05f", average);
		//
		// 	ESP_ERROR_CHECK(a_mqtt_publish("/tests/esp32/average", buffer));
		// }

		active_slot = (active_slot + 1) % 2;

		vTaskSuspend(NULL);
	}
}
