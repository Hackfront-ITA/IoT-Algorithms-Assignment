#ifndef A_TASKS_TASKS_H
#define A_TASKS_TASKS_H

#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

typedef struct {
	size_t num_samples;
	float *adc_data;
	float sampling_freq;
} task_args_t;

void task_data_read(task_args_t *task_args);
void task_data_process(task_args_t *task_args);

extern TaskHandle_t th_data_read;
extern TaskHandle_t th_data_process;

#endif /* end of include guard: A_TASKS_TASKS_H */
