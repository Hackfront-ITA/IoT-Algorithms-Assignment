#include <math.h>
#include <stdio.h>

#include <esp_dsp.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "utils.h"

#define N_SAMPLES 2048

static const char *TAG = "a_fft";

// static SemaphoreHandle_t buf_sem;
static size_t fft_size = 0;
static float *window = NULL;
static float *buffer = NULL;

esp_err_t a_fft_init(void) {
	esp_err_t result;
	result = dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
	if (result != ESP_OK) {
		ESP_LOGE(TAG, "dsps_fft2r_init_fc32");
		return result;
	}

	// buf_sem = xSemaphoreCreateBinary();
	// if (buf_sem == NULL) {
	// 	ESP_LOGE(TAG, "Cannot create semaphore buf_sem");
	// 	return ESP_FAIL;
	// }

	return ESP_OK;
}

esp_err_t a_fft_set_size(size_t size) {
	// if (!xSemaphoreTake(buf_sem, C_DELAY_MS(1000))) {
	// 	ESP_LOGE(TAG, "Cannot take FFT buffer lock");
	// 	return ESP_FAIL;
	// }

	fft_size = size;

	if (buffer != NULL) {
		free(buffer);
	}

	if (window != NULL) {
		free(window);
	}

	buffer = malloc(fft_size * sizeof(float));
	if (buffer == NULL) {
		ESP_LOGE(TAG, "malloc buffer");
		return ESP_FAIL;
	}

	// xSemaphoreGive(buf_sem);

	window = malloc(fft_size * sizeof(float));
	if (window == NULL) {
		ESP_LOGE(TAG, "malloc window");
		return ESP_FAIL;
	}

	dsps_wind_hann_f32(window, fft_size);

	return ESP_OK;
}

// esp_err_t a_fft_prepare(float *input) {
//   if (buffer == NULL || window == NULL) {
//     return ESP_FAIL;
//   }
//
//   if (!xSemaphoreTake(buf_sem, C_DELAY_MS(1000))) {
//     ESP_LOGE(TAG, "Cannot take FFT buffer lock");
//     return ESP_FAIL;
//   }
//
//   for (int i = 0 ; i < fft_size; i++) {
//     buffer[i * 2 + 0] = input[i] * window[i];  // Re([i])
//     buffer[i * 2 + 1] = 0;  // Im([i])
//   }
//
//   xSemaphoreGive(buf_sem);
//
//   return ESP_OK;
// }

esp_err_t a_fft_execute(float *input, float *output) {
	esp_err_t result;

	if (buffer == NULL || window == NULL) {
		ESP_LOGE(TAG, "FFT not initialized yet");
		return ESP_FAIL;
	}

	// if (!xSemaphoreTake(buf_sem, C_DELAY_MS(1000))) {
	// 	ESP_LOGE(TAG, "Cannot take FFT buffer lock");
	// 	return ESP_FAIL;
	// }

	for (int i = 0 ; i < fft_size; i++) {
		buffer[i * 2 + 0] = input[i] * window[i];  // Re([i])
		buffer[i * 2 + 1] = 0;  // Im([i])
	}

	result = dsps_fft2r_fc32(buffer, fft_size);
	if (result != ESP_OK) {
		ESP_LOGE(TAG, "dsps_fft2r_fc32");
		return result;
	}

	result = dsps_bit_rev_fc32(buffer, fft_size);
	if (result != ESP_OK) {
		ESP_LOGE(TAG, "dsps_bit_rev_fc32");
		return result;
	}

	result = dsps_cplx2reC_fc32(buffer, fft_size);
	if (result != ESP_OK) {
		ESP_LOGE(TAG, "dsps_cplx2reC_fc32");
		return result;
	}

	for (int i = 0; i < (fft_size / 2); i++) {
		float abs = C_ABS(buffer[i * 2 + 0], buffer[i * 2 + 1]);
		float norm = abs / fft_size;

		output[i] = C_LIN_TO_DB(norm);
	}

	// xSemaphoreGive(buf_sem);

	return ESP_OK;
}
