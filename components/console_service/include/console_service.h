#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "app_types.h"

esp_err_t console_service_start(const alarm_limits_t *limits, SemaphoreHandle_t limits_mutex);
