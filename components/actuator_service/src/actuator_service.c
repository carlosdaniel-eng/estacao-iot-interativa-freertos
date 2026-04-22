#include "actuator_service.h"

#include "driver/ledc.h"
#include "app_config.h"

static bool s_buzzer_active;

static void set_rgb_channel(ledc_channel_t channel, uint32_t duty)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
}

esp_err_t actuator_service_init(void)
{
    ledc_timer_config_t rgb_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_timer_config_t buzzer_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_1,
        .freq_hz = APP_BUZZER_FREQUENCY_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_channel_config_t rgb_channels[] = {
        {.gpio_num = APP_RGB_RED_PIN, .speed_mode = LEDC_LOW_SPEED_MODE, .channel = LEDC_CHANNEL_0, .intr_type = LEDC_INTR_DISABLE, .timer_sel = LEDC_TIMER_0, .duty = 0, .hpoint = 0},
        {.gpio_num = APP_RGB_GREEN_PIN, .speed_mode = LEDC_LOW_SPEED_MODE, .channel = LEDC_CHANNEL_1, .intr_type = LEDC_INTR_DISABLE, .timer_sel = LEDC_TIMER_0, .duty = 0, .hpoint = 0},
        {.gpio_num = APP_RGB_BLUE_PIN, .speed_mode = LEDC_LOW_SPEED_MODE, .channel = LEDC_CHANNEL_2, .intr_type = LEDC_INTR_DISABLE, .timer_sel = LEDC_TIMER_0, .duty = 0, .hpoint = 0},
        {.gpio_num = APP_BUZZER_PIN, .speed_mode = LEDC_LOW_SPEED_MODE, .channel = LEDC_CHANNEL_3, .intr_type = LEDC_INTR_DISABLE, .timer_sel = LEDC_TIMER_1, .duty = 0, .hpoint = 0},
    };

    ESP_ERROR_CHECK(ledc_timer_config(&rgb_timer));
    ESP_ERROR_CHECK(ledc_timer_config(&buzzer_timer));

    for (size_t i = 0; i < sizeof(rgb_channels) / sizeof(rgb_channels[0]); ++i) {
        ESP_ERROR_CHECK(ledc_channel_config(&rgb_channels[i]));
    }

    s_buzzer_active = false;
    return ESP_OK;
}

void actuator_service_set_rgb(const rgb_led_state_t *state)
{
    if (state == NULL) {
        return;
    }

    set_rgb_channel(LEDC_CHANNEL_0, state->red);
    set_rgb_channel(LEDC_CHANNEL_1, state->green);
    set_rgb_channel(LEDC_CHANNEL_2, state->blue);
}

void actuator_service_set_buzzer(bool enabled)
{
    s_buzzer_active = enabled;
    set_rgb_channel(LEDC_CHANNEL_3, enabled ? APP_BUZZER_DUTY : 0);
}

bool actuator_service_is_buzzer_active(void)
{
    return s_buzzer_active;
}
