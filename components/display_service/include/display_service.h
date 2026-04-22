#pragma once

#include "esp_err.h"
#include "app_types.h"

esp_err_t display_service_init(void);
esp_err_t display_service_render(const ui_state_t *state);
