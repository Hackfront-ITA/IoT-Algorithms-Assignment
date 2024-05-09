#include <stddef.h>
#include <stdio.h>

float calc_ps_factor(float *data, size_t len, float threshold) {
	size_t max_index = 0;
	for (size_t i = 0; i < len; i++) {
		if (data[i] >= threshold) {
			max_index = i;
		}
	}

	printf("max_index = %d, max_value = %f\n", max_index, data[max_index]);

	float factor = max_index;
	factor /= len;

	return factor;
}
