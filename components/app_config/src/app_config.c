#include "app_config.h"

#include <stdio.h>

const gpio_num_t APP_KEYPAD_ROWS[4] = {
    GPIO_NUM_0,
    GPIO_NUM_1,
    GPIO_NUM_3,
    GPIO_NUM_4,
};

const gpio_num_t APP_KEYPAD_COLS[4] = {
    GPIO_NUM_20,
    GPIO_NUM_21,
    GPIO_NUM_5,
    GPIO_NUM_19,
};

void app_config_get_default_limits(alarm_limits_t *limits)
{
    if (limits == NULL) {
        return;
    }

    limits->temperature_min = 20.0f;
    limits->temperature_max = 30.0f;
    limits->humidity_min = 30.0f;
    limits->humidity_max = 80.0f;
    limits->luminosity_min = 100;
    limits->luminosity_max = 900;
}

void app_config_build_feed_topic(const char *feed_name, char *buffer, size_t buffer_size)
{
    if (feed_name == NULL || buffer == NULL || buffer_size == 0) {
        return;
    }

    snprintf(buffer, buffer_size, "%s/feeds/%s", APP_ADAFRUIT_IO_USERNAME, feed_name);
}
