#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "app_types.h"

alarm_flags_t alarm_service_evaluate(const sensor_sample_t *sample, const alarm_limits_t *limits);
void alarm_service_describe(alarm_flags_t flags, char *buffer, size_t buffer_size);
void alarm_service_adjust_limit(alarm_limits_t *limits, limit_field_t field, int direction);
const char *alarm_service_limit_name(limit_field_t field);
void alarm_service_limit_value_to_string(const alarm_limits_t *limits, limit_field_t field, char *buffer, size_t buffer_size);
