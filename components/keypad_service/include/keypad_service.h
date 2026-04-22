#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_err.h"

esp_err_t keypad_service_start(QueueHandle_t key_queue);
