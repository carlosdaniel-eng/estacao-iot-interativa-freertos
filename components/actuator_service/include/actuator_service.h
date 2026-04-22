#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "app_types.h"

esp_err_t actuator_service_init(void);
void actuator_service_set_rgb(const rgb_led_state_t *state);
void actuator_service_set_buzzer(bool enabled);
bool actuator_service_is_buzzer_active(void);
