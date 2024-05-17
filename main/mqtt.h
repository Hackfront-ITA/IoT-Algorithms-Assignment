#ifndef A_MQTT_H
#define A_MQTT_H

#include <esp_err.h>

esp_err_t a_mqtt_start(void);
esp_err_t a_mqtt_stop(void);
esp_err_t a_mqtt_publish(const char *topic, const char *data);

#endif /* end of include guard: A_MQTT_H */
