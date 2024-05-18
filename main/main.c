#include <math.h>
#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_dsp.h>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include "tasks/tasks.h"
#include "adapt.h"
#include "adc.h"
#include "fft.h"
#include "mqtt.h"
#include "network.h"
#include "utils.h"

#define A_DURATION  1000

#define A_TASK_STACK_SIZE  1024

TaskHandle_t th_data_read = NULL;
TaskHandle_t th_data_process = NULL;

static const char *TAG = "App main";

static task_args_t task_args;

void app_main(void) {
	ESP_LOGI(TAG, "App start");

	ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

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
	size_t num_samples = A_DURATION * sampling_freq / 1000.0;

	ESP_LOGI(TAG, "new max frequency = %f", max_freq);
	ESP_LOGI(TAG, "new num_samples = %d", num_samples);

	ESP_LOGI(TAG, "calloc(%d, %d)", 4 * num_samples, sizeof(float));
	float *adc_data = calloc(4 * num_samples, sizeof(float));
	if (adc_data == NULL) {
		ESP_LOGE(TAG, "Error allocating adc_data (after adapt)");
		return;
	}

	task_args.sampling_freq = sampling_freq;
	task_args.num_samples = num_samples;
	task_args.adc_data = adc_data;

	ESP_LOGI(TAG, "Create data read task\n");
	xTaskCreatePinnedToCore(task_data_read, "Data read task", A_TASK_STACK_SIZE, &task_args, 10, &th_data_read, 1);

	ESP_LOGI(TAG, "Create data processing task\n");
	xTaskCreate(task_data_process, "Data processing task", A_TASK_STACK_SIZE, &task_args, 10, &th_data_process);

	// ESP_LOGI(TAG, "Connect to network");
	// ESP_ERROR_CHECK(a_network_connect());
	//
	// ESP_LOGI(TAG, "Connect to MQTT server");
	// ESP_ERROR_CHECK(a_mqtt_start());

	// ESP_LOGI(TAG, "Running...");

	// ESP_LOGI(TAG, "Disconnect from MQTT server");
	// ESP_ERROR_CHECK(a_mqtt_stop());
	//
	// ESP_LOGI(TAG, "Disconnect from network");
	// ESP_ERROR_CHECK(a_network_disconnect());

	// free(adc_data);

	return;
}
