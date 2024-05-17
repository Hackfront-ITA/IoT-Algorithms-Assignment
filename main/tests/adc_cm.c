#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_adc/adc_continuous.h"

#define A_ADC_UNIT                    ADC_UNIT_1
#define A_ADC_CONV_MODE               ADC_CONV_SINGLE_UNIT_1
#define A_ADC_ATTEN                   ADC_ATTEN_DB_12
#define A_ADC_BIT_WIDTH               SOC_ADC_DIGI_MAX_BITWIDTH

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define A_ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define A_ADC_GET_CHANNEL(p_data)     ((p_data)->type1.channel)
#define A_ADC_GET_DATA(p_data)        ((p_data)->type1.data)
#else
#define A_ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE2
#define A_ADC_GET_CHANNEL(p_data)     ((p_data)->type2.channel)
#define A_ADC_GET_DATA(p_data)        ((p_data)->type2.data)
#endif

#define A_READ_LEN                    256

#define A_ADC_CHANNEL  ADC_CHANNEL_4

static TaskHandle_t s_task_handle;
static const char *TAG = "EXAMPLE";

static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data) {
  BaseType_t mustYield = pdFALSE;
  vTaskNotifyGiveFromISR(s_task_handle, &mustYield);

  return (mustYield == pdTRUE);
}

static void continuous_adc_init(adc_channel_t channel, uint8_t channel_num, adc_continuous_handle_t *out_handle) {
  adc_continuous_handle_t handle = NULL;

  adc_continuous_handle_cfg_t adc_config = {
    .max_store_buf_size = 1024,
    .conv_frame_size = A_READ_LEN,
  };
  ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

  adc_continuous_config_t dig_cfg = {
    .sample_freq_hz = 20 * 1000,
    .conv_mode = A_ADC_CONV_MODE,
    .format = A_ADC_OUTPUT_TYPE,
  };

  adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = { 0 };
  dig_cfg.pattern_num = channel_num;
  adc_pattern[0].atten = A_ADC_ATTEN;
  adc_pattern[0].channel = channel & 0x7;
  adc_pattern[0].unit = A_ADC_UNIT;
  adc_pattern[0].bit_width = A_ADC_BIT_WIDTH;

  ESP_LOGI(TAG, "adc_pattern[0].atten is :%" PRIx8, adc_pattern[0].atten);
  ESP_LOGI(TAG, "adc_pattern[0].channel is :%" PRIx8, adc_pattern[0].channel);
  ESP_LOGI(TAG, "adc_pattern[0].unit is :%" PRIx8, adc_pattern[0].unit);

  dig_cfg.adc_pattern = adc_pattern;
  ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));

  adc_continuous_evt_cbs_t cbs = {
    .on_conv_done = s_conv_done_cb,
    .on_pool_ovf = NULL,
  };
  ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(handle, &cbs, NULL));
}

void app_main(void) {
  uint32_t ret_num = 0;
  uint8_t result[A_READ_LEN] = { 0 };
  memset(result, 0xcc, A_READ_LEN);

  s_task_handle = xTaskGetCurrentTaskHandle();

  adc_continuous_handle_t handle = NULL;
  continuous_adc_init(A_ADC_CHANNEL, 1, &handle);

  ESP_ERROR_CHECK(adc_continuous_start(handle));

  char unit[] = "ADC_UNIT_1";

  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

  while (1) {
    esp_err_t ret = adc_continuous_read(handle, result, A_READ_LEN, &ret_num, 100);
    if (ret == ESP_ERR_TIMEOUT) {
      break;
    } else if (ret != ESP_OK) {
      break;
    }

    ESP_LOGI("TASK", "ret is %x, ret_num is %" PRIu32 " bytes", ret, ret_num);

    for (int i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES) {
      adc_digi_output_data_t *p = (adc_digi_output_data_t*)&result[i];
      uint32_t chan_num = A_ADC_GET_CHANNEL(p);
      uint32_t data = A_ADC_GET_DATA(p);
      
      if (chan_num < SOC_ADC_CHANNEL_NUM(A_ADC_UNIT)) {
        // ESP_LOGI(TAG, "Unit: %s, Channel: %" PRIu32 ", Value: %" PRIx32, unit, chan_num, data);
      } else {
        ESP_LOGW(TAG, "Invalid data [%s_%" PRIu32 "_%" PRIx32 "]", unit, chan_num, data);
      }
    }
    vTaskDelay(1);
  }

  ESP_ERROR_CHECK(adc_continuous_stop(handle));
  ESP_ERROR_CHECK(adc_continuous_deinit(handle));
}
