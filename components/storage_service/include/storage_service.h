#pragma once

#include "esp_err.h"
#include "app_types.h"

esp_err_t storage_service_init(void);
esp_err_t storage_service_load_limits(alarm_limits_t *limits);
esp_err_t storage_service_save_limits(const alarm_limits_t *limits);
esp_err_t storage_service_append_alarm(const alarm_log_entry_t *entry);
void storage_service_print_alarm_logs(void);
esp_err_t storage_service_clear_alarm_logs(void);
