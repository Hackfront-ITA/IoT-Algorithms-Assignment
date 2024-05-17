#ifndef A_ADC_H
#define A_ADC_H

#include <stdint.h>

#include <esp_err.h>

esp_err_t a_adc_init(void);
esp_err_t a_adc_set_sampling_freq(uint32_t sampling_freq);
esp_err_t a_adc_start(void);
esp_err_t a_adc_stop(void);
esp_err_t a_adc_collect_samples(float *buffer, size_t length, float flush_freq);

#endif /* end of include guard: A_ADC_H */
