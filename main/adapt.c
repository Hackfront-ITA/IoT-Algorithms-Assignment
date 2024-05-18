#include <math.h>
#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_dsp.h>
#include <esp_err.h>
#include <esp_log.h>

#include "adc.h"
#include "fft.h"
#include "utils.h"

#define A_ADAPT_NUM_SAMPLES      8192
#define A_ADAPT_SAMPLING_FREQ    (20 * 1000)
#define A_ADAPT_FLUSH_FREQ       (A_ADAPT_SAMPLING_FREQ * 1.2)
#define A_ADAPT_TOLERANCE        1.03

#define A_FFT_THRESHOLD   60

static const char *TAG = "Adapt";

float sense_sampling_freq(void) {
	ESP_ERROR_CHECK(a_fft_init());
	ESP_ERROR_CHECK(a_fft_set_size(A_ADAPT_NUM_SAMPLES));

	float *adc_data = calloc(A_ADAPT_NUM_SAMPLES, sizeof(float));
	if (adc_data == NULL) {
		ESP_LOGE(TAG, "Error allocating adc_data (before adapt)");
		return 0.0;
	}

	float *fft_data = calloc(A_ADAPT_NUM_SAMPLES / 2, sizeof(float));
	if (fft_data == NULL) {
		ESP_LOGE(TAG, "Error allocating fft_data");
		return 0.0;
	}

	ESP_ERROR_CHECK(a_adc_set_sampling_freq(A_ADAPT_SAMPLING_FREQ));

	ESP_ERROR_CHECK(a_adc_start());
	ESP_ERROR_CHECK(a_adc_collect_samples(adc_data, A_ADAPT_NUM_SAMPLES, A_ADAPT_FLUSH_FREQ));
	ESP_ERROR_CHECK(a_adc_stop());

	remove_dc_offset(adc_data, A_ADAPT_NUM_SAMPLES);

	ESP_LOGI(TAG, "*** View ADC data ***");
	if (esp_log_level_get(TAG) == ESP_LOG_INFO) {
		dsps_view(adc_data, 128, 128, 10, -50, +50, '@');
	}

	ESP_ERROR_CHECK(a_fft_execute(adc_data, fft_data));

	ESP_LOGI(TAG, "*** View FFT output ***");
	if (esp_log_level_get(TAG) == ESP_LOG_INFO) {
		dsps_view(fft_data, A_ADAPT_NUM_SAMPLES / 2, 128, 20, -100, 100, '*');
	}

	float factor = calc_powersave_factor(fft_data, A_ADAPT_NUM_SAMPLES / 2, A_FFT_THRESHOLD);
	ESP_LOGI(TAG, "factor = %f", factor);
	if (factor < 0.01) {
		return 0.0;
	}

	float sampling_freq = floor(A_ADAPT_SAMPLING_FREQ * factor * A_ADAPT_TOLERANCE);
	ESP_LOGI(TAG, "sensed sampling frequency: %.02f Hz", sampling_freq);

	sampling_freq = fmax(sampling_freq, SOC_ADC_SAMPLE_FREQ_THRES_LOW);
	sampling_freq = fmin(sampling_freq, SOC_ADC_SAMPLE_FREQ_THRES_HIGH);
	ESP_LOGI(TAG, "actual ADC sampling frequency: %.02f Hz", sampling_freq);

	free(adc_data);
	free(fft_data);

	ESP_ERROR_CHECK(a_fft_destroy());

	return sampling_freq;
}
