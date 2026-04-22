#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define APP_MAX_STATUS_TEXT 32
#define APP_MAX_ALARM_TEXT 96

typedef struct {
    float temperature_c;
    float humidity_pct;
    int luminosity_raw;
    bool dht_valid;
    int64_t timestamp_ms;
} sensor_sample_t;

typedef struct {
    float temperature_min;
    float temperature_max;
    float humidity_min;
    float humidity_max;
    int luminosity_min;
    int luminosity_max;
} alarm_limits_t;

typedef enum {
    ALARM_FLAG_NONE = 0,
    ALARM_FLAG_TEMP_LOW = 1 << 0,
    ALARM_FLAG_TEMP_HIGH = 1 << 1,
    ALARM_FLAG_HUM_LOW = 1 << 2,
    ALARM_FLAG_HUM_HIGH = 1 << 3,
    ALARM_FLAG_LUX_LOW = 1 << 4,
    ALARM_FLAG_LUX_HIGH = 1 << 5,
    ALARM_FLAG_SENSOR_FAULT = 1 << 6,
} alarm_flags_t;

typedef enum {
    LIMIT_FIELD_TEMP_MIN = 0,
    LIMIT_FIELD_TEMP_MAX,
    LIMIT_FIELD_LUX_MIN,
    LIMIT_FIELD_LUX_MAX,
    LIMIT_FIELD_HUM_MIN,
    LIMIT_FIELD_HUM_MAX,
    LIMIT_FIELD_COUNT,
} limit_field_t;

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} rgb_led_state_t;

typedef struct {
    char timestamp[24];
    sensor_sample_t sample;
    alarm_flags_t flags;
} alarm_log_entry_t;

typedef struct {
    bool wifi_connected;
    bool mqtt_connected;
    bool time_synced;
    bool buzzer_active;
    sensor_sample_t sample;
    alarm_limits_t limits;
    alarm_flags_t active_alarms;
    rgb_led_state_t led_state;
    uint8_t selected_limit;
    char status_text[APP_MAX_STATUS_TEXT];
} ui_state_t;
