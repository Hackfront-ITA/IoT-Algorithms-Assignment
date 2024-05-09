#ifndef N_UTILS_H
#define N_UTILS_H

#include <math.h>

#define A_ALIGN_16     __attribute__((aligned(16)))

#define C_ABS(a,b)      ((a * a) + (b * b))
#define C_DELAY_MS(v)   ((v) / portTICK_PERIOD_MS)
#define C_LIN_TO_DB(l)  (10 * log10f(l))

float calc_ps_factor(float *data, size_t len, float threshold);

#endif /* end of include guard: N_UTILS_H */
