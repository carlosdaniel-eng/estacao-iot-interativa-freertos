#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "app_types.h"

typedef void (*mqtt_rgb_callback_t)(const rgb_led_state_t *state);

esp_err_t mqtt_service_start(mqtt_rgb_callback_t callback);
bool mqtt_service_is_connected(void);
esp_err_t mqtt_service_publish_sample(const sensor_sample_t *sample);
esp_err_t mqtt_service_publish_alarm(alarm_flags_t flags);
