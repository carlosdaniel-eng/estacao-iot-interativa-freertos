#pragma once

#include "esp_err.h"
#include "app_types.h"

esp_err_t sensor_service_init(void);
esp_err_t sensor_service_read(sensor_sample_t *sample);
