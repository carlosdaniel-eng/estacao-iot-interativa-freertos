#include "alarm_service.h"

#include <stdio.h>
#include <string.h>

static void append_text(char *buffer, size_t buffer_size, const char *text)
{
    size_t used = strlen(buffer);
    if (used >= buffer_size - 1) {
        return;
    }

    snprintf(buffer + used, buffer_size - used, "%s%s", used == 0 ? "" : " ", text);
}

alarm_flags_t alarm_service_evaluate(const sensor_sample_t *sample, const alarm_limits_t *limits)
{
    alarm_flags_t flags = ALARM_FLAG_NONE;

    if (sample == NULL || limits == NULL) {
        return ALARM_FLAG_SENSOR_FAULT;
    }

    if (!sample->dht_valid) {
        flags |= ALARM_FLAG_SENSOR_FAULT;
    } else {
        if (sample->temperature_c < limits->temperature_min) {
            flags |= ALARM_FLAG_TEMP_LOW;
        }
        if (sample->temperature_c > limits->temperature_max) {
            flags |= ALARM_FLAG_TEMP_HIGH;
        }
        if (sample->humidity_pct < limits->humidity_min) {
            flags |= ALARM_FLAG_HUM_LOW;
        }
        if (sample->humidity_pct > limits->humidity_max) {
            flags |= ALARM_FLAG_HUM_HIGH;
        }
    }

    if (sample->luminosity_raw < limits->luminosity_min) {
        flags |= ALARM_FLAG_LUX_LOW;
    }
    if (sample->luminosity_raw > limits->luminosity_max) {
        flags |= ALARM_FLAG_LUX_HIGH;
    }

    return flags;
}

void alarm_service_describe(alarm_flags_t flags, char *buffer, size_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0) {
        return;
    }

    buffer[0] = '\0';

    if (flags == ALARM_FLAG_NONE) {
        snprintf(buffer, buffer_size, "NONE");
        return;
    }

    if (flags & ALARM_FLAG_TEMP_LOW) {
        append_text(buffer, buffer_size, "TEMP LOW");
    }
    if (flags & ALARM_FLAG_TEMP_HIGH) {
        append_text(buffer, buffer_size, "TEMP HIGH");
    }
    if (flags & ALARM_FLAG_HUM_LOW) {
        append_text(buffer, buffer_size, "HUM LOW");
    }
    if (flags & ALARM_FLAG_HUM_HIGH) {
        append_text(buffer, buffer_size, "HUM HIGH");
    }
    if (flags & ALARM_FLAG_LUX_LOW) {
        append_text(buffer, buffer_size, "LUX LOW");
    }
    if (flags & ALARM_FLAG_LUX_HIGH) {
        append_text(buffer, buffer_size, "LUX HIGH");
    }
    if (flags & ALARM_FLAG_SENSOR_FAULT) {
        append_text(buffer, buffer_size, "SENSOR FAIL");
    }
}

void alarm_service_adjust_limit(alarm_limits_t *limits, limit_field_t field, int direction)
{
    if (limits == NULL) {
        return;
    }

    switch (field) {
    case LIMIT_FIELD_TEMP_MIN:
        limits->temperature_min += 0.5f * direction;
        if (limits->temperature_min > limits->temperature_max - 0.5f) {
            limits->temperature_min = limits->temperature_max - 0.5f;
        }
        break;
    case LIMIT_FIELD_TEMP_MAX:
        limits->temperature_max += 0.5f * direction;
        if (limits->temperature_max < limits->temperature_min + 0.5f) {
            limits->temperature_max = limits->temperature_min + 0.5f;
        }
        break;
    case LIMIT_FIELD_LUX_MIN:
        limits->luminosity_min += 10 * direction;
        if (limits->luminosity_min > limits->luminosity_max - 10) {
            limits->luminosity_min = limits->luminosity_max - 10;
        }
        break;
    case LIMIT_FIELD_LUX_MAX:
        limits->luminosity_max += 10 * direction;
        if (limits->luminosity_max < limits->luminosity_min + 10) {
            limits->luminosity_max = limits->luminosity_min + 10;
        }
        break;
    case LIMIT_FIELD_HUM_MIN:
        limits->humidity_min += 0.5f * direction;
        if (limits->humidity_min > limits->humidity_max - 0.5f) {
            limits->humidity_min = limits->humidity_max - 0.5f;
        }
        break;
    case LIMIT_FIELD_HUM_MAX:
        limits->humidity_max += 0.5f * direction;
        if (limits->humidity_max < limits->humidity_min + 0.5f) {
            limits->humidity_max = limits->humidity_min + 0.5f;
        }
        break;
    default:
        break;
    }
}

const char *alarm_service_limit_name(limit_field_t field)
{
    switch (field) {
    case LIMIT_FIELD_TEMP_MIN:
        return "TMIN";
    case LIMIT_FIELD_TEMP_MAX:
        return "TMAX";
    case LIMIT_FIELD_LUX_MIN:
        return "LMIN";
    case LIMIT_FIELD_LUX_MAX:
        return "LMAX";
    case LIMIT_FIELD_HUM_MIN:
        return "HMIN";
    case LIMIT_FIELD_HUM_MAX:
        return "HMAX";
    default:
        return "UNKN";
    }
}

void alarm_service_limit_value_to_string(const alarm_limits_t *limits, limit_field_t field, char *buffer, size_t buffer_size)
{
    if (limits == NULL || buffer == NULL || buffer_size == 0) {
        return;
    }

    switch (field) {
    case LIMIT_FIELD_TEMP_MIN:
        snprintf(buffer, buffer_size, "%.1fC", limits->temperature_min);
        break;
    case LIMIT_FIELD_TEMP_MAX:
        snprintf(buffer, buffer_size, "%.1fC", limits->temperature_max);
        break;
    case LIMIT_FIELD_LUX_MIN:
        snprintf(buffer, buffer_size, "%d", limits->luminosity_min);
        break;
    case LIMIT_FIELD_LUX_MAX:
        snprintf(buffer, buffer_size, "%d", limits->luminosity_max);
        break;
    case LIMIT_FIELD_HUM_MIN:
        snprintf(buffer, buffer_size, "%.1f%%", limits->humidity_min);
        break;
    case LIMIT_FIELD_HUM_MAX:
        snprintf(buffer, buffer_size, "%.1f%%", limits->humidity_max);
        break;
    default:
        snprintf(buffer, buffer_size, "--");
        break;
    }
}
