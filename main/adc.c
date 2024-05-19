#include <math.h>
#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_adc/adc_continuous.h>

#include "utils.h"

#define A_ADC_UNIT           ADC_UNIT_1
#define A_ADC_CHANNEL        ADC_CHANNEL_4
#define A_ADC_ATTEN          ADC_ATTEN_DB_12
#define A_ADC_BIT_WIDTH      SOC_ADC_DIGI_MAX_BITWIDTH
#define A_ADC_CONV_MODE      ADC_CONV_SINGLE_UNIT_1

#define A_ADC_SAMPLING_FREQ  20000
#define A_ADC_MAX_VAL        4096

#define A_ADC_READ_TIMEOUT   1000
#define A_ADC_BUF_SIZE       4096

#define A_ADC_READ_LEN       1024

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
  #define A_ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE1
  #define A_ADC_GET_CHANNEL(p_data)     ((p_data)->type1.channel)
  #define A_ADC_GET_DATA(p_data)        ((p_data)->type1.data)
#else
  #define A_ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE2
  #define A_ADC_GET_CHANNEL(p_data)     ((p_data)->type2.channel)
  #define A_ADC_GET_DATA(p_data)        ((p_data)->type2.data)
#endif

static const char *TAG = "ADC module";

static volatile uint32_t overflow_count = 0;
static uint8_t a_result[A_ADC_READ_LEN];

static adc_continuous_handle_t a_handle = NULL;

static adc_digi_pattern_config_t a_dpc = {
  .unit = A_ADC_UNIT,
  .channel = A_ADC_CHANNEL & 0x07,
  .atten = A_ADC_ATTEN,
  .bit_width = A_ADC_BIT_WIDTH,
};

static adc_continuous_handle_cfg_t a_handle_cfg = {
  .max_store_buf_size = A_ADC_BUF_SIZE,
  .conv_frame_size = A_ADC_READ_LEN,
};

static adc_continuous_config_t a_config = {
  .sample_freq_hz = A_ADC_SAMPLING_FREQ,
  .conv_mode = A_ADC_CONV_MODE,
  .format = A_ADC_OUTPUT_TYPE,
  .pattern_num = 1,
  .adc_pattern = &a_dpc,
};

static bool IRAM_ATTR a_conv_done_cb(adc_continuous_handle_t handle,
    const adc_continuous_evt_data_t *edata, void *user_data)
{
  return true;
}

static bool IRAM_ATTR a_pool_ovf_cb(adc_continuous_handle_t handle,
    const adc_continuous_evt_data_t *edata, void *user_data)
{
	overflow_count++;

  return true;
}

static adc_continuous_evt_cbs_t a_evt_cbs = {
  .on_conv_done = a_conv_done_cb,
  .on_pool_ovf = a_pool_ovf_cb,
};

esp_err_t a_adc_init(void) {
  esp_err_t err;

  err = adc_continuous_new_handle(&a_handle_cfg, &a_handle);
  if (err != ESP_OK) {
    return err;
  }

  err = adc_continuous_register_event_callbacks(a_handle, &a_evt_cbs, NULL);
  if (err != ESP_OK) {
    return err;
  }

  err = adc_continuous_config(a_handle, &a_config);
  if (err != ESP_OK) {
    return err;
  }

  return ESP_OK;
}

esp_err_t a_adc_set_sampling_freq(uint32_t sampling_freq) {
  esp_err_t err;

  a_config.sample_freq_hz = sampling_freq;

  err = adc_continuous_config(a_handle, &a_config);
  if (err != ESP_OK) {
    return err;
  }

  return ESP_OK;
}

esp_err_t a_adc_start(void) {
  return adc_continuous_start(a_handle);
}

esp_err_t a_adc_stop(void) {
  return adc_continuous_stop(a_handle);
}

esp_err_t a_adc_collect_samples(float *buffer, size_t length, float flush_freq) {
  size_t index = 0;
  uint16_t waiting_time = floor((A_ADC_READ_LEN / 4) * 1000.0 / flush_freq);
  waiting_time = C_MAX(waiting_time, 1);
  ESP_LOGI(TAG, "waiting_time = %hu", waiting_time);

	while (index < length) {
		vTaskDelay(C_DELAY_MS(waiting_time));

		uint32_t rsize = 0;

		esp_err_t err = adc_continuous_read(a_handle, a_result, A_ADC_READ_LEN,
      &rsize, A_ADC_READ_TIMEOUT);

		if (err == ESP_ERR_TIMEOUT) {
      ESP_LOGE(TAG, "ADC read timeout");
			return err;
		} else if (err != ESP_OK) {
      ESP_LOGE(TAG, "Other ADC error: %d", err);
			return err;
		}

		for (int i = 0; i < rsize; i += SOC_ADC_DIGI_RESULT_BYTES) {
			adc_digi_output_data_t *ptr = (adc_digi_output_data_t*)(&a_result[i]);
			uint16_t channel = A_ADC_GET_CHANNEL(ptr);
			uint16_t data = A_ADC_GET_DATA(ptr);

			if (channel == A_ADC_CHANNEL) {
				float adc_value = data;
				adc_value /= A_ADC_MAX_VAL;
				adc_value *= 100;
				buffer[index] = adc_value;

				index++;
			} else {
				ESP_LOGW(TAG, "Invalid data [ADC_UNIT_?, %" PRIu16 "_%" PRIx16 "]",
          channel, data);
			}
		}
	}

	ESP_LOGI(TAG, "overflow_count = %lu", overflow_count);
	overflow_count = 0;

  return ESP_OK;
}
