#include <stddef.h>
#include <stdio.h>

#include <esp_log.h>
#include <esp_tls.h>

static const char *TAG = "Utils";

float calc_powersave_factor(float *data, size_t len, float threshold) {
	size_t max_index = 0;
	for (size_t i = 0; i < len; i++) {
		if (data[i] >= threshold) {
			max_index = i;
		}
	}

	ESP_LOGI(TAG, "max_index = %d, max_value = %f", max_index, data[max_index]);

	float factor = max_index;
	factor /= len;

	return factor;
}

float calc_average(float *data, size_t len) {
	float average = 0.0;

	for (size_t i = 0; i < len; i++) {
		average += data[i];
	}

	average /= len;

	return average;
}

void print_ciphers(void) {
	const int *clist = esp_tls_get_ciphersuites_list();

	int i = 0;
	while (clist[i] != 0) {
		printf("clist[%02d] = %04x\n", i, clist[i]);
		i++;
	}
}

void remove_dc_offset(float *data, size_t len) {
	float average = calc_average(data, len);

	for (size_t i = 0; i < len; i++) {
		data[i] -= average;
	}
}
