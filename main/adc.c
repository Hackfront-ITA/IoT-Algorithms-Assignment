#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

// ADC1_CHANNEL_0 -> GPIO 36
// ADC1_CHANNEL_1 -> GPIO 37
// ADC1_CHANNEL_2 -> GPIO 38
// ADC1_CHANNEL_3 -> GPIO 39
// ADC1_CHANNEL_4 -> GPIO 32
// ADC1_CHANNEL_5 -> GPIO 33
// ADC1_CHANNEL_6 -> GPIO 34
// ADC1_CHANNEL_7 -> GPIO 35

// ADC_ATTEN_DB_0   -> 100 mV -  950 mV
// ADC_ATTEN_DB_2_5 -> 100 mV - 1250 mV
// ADC_ATTEN_DB_6   -> 100 mV - 1750 mV
// ADC_ATTEN_DB_11  -> 100 mV - 2450 mV

// ADC_WIDTH_9Bit
// ADC_WIDTH_10Bit
// ADC_WIDTH_11Bit
// ADC_WIDTH_12Bit

#define A_ADC_CHANNEL  ADC1_CHANNEL_4
#define A_ADC_ATTEN    ADC_ATTEN_DB_11
#define A_ADC_WIDTH    ADC_WIDTH_BIT_DEFAULT

static esp_adc_cal_characteristics_t adc1_chars;

void app_main(void) {
  esp_adc_cal_characterize(ADC_UNIT_1, A_ADC_ATTEN, A_ADC_WIDTH, 0, &adc1_chars);
  ESP_ERROR_CHECK(adc1_config_width(A_ADC_WIDTH));
  ESP_ERROR_CHECK(adc1_config_channel_atten(A_ADC_CHANNEL, A_ADC_ATTEN));

  while (1) {
    uint32_t adc_value = adc1_get_raw(A_ADC_CHANNEL);
    printf("Raw value: %d LSB", adc_value);

    // uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_value, &adc1_chars);
    // printf("Voltage: %d mV", voltage);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  return;
}
