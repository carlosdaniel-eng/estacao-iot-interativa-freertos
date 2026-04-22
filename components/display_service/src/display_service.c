#include "display_service.h"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "driver/i2c.h"
#include "esp_check.h"
#include "app_config.h"
#include "alarm_service.h"

static const char *TAG = "display_service";
static uint8_t s_framebuffer[APP_OLED_WIDTH * APP_OLED_HEIGHT / 8];

static esp_err_t oled_command(uint8_t command)
{
    uint8_t payload[2] = {0x00, command};
    return i2c_master_write_to_device(
        APP_OLED_I2C_PORT,
        APP_OLED_I2C_ADDRESS,
        payload,
        sizeof(payload),
        pdMS_TO_TICKS(100));
}

static const uint8_t *glyph_for_char(char c)
{
    static const uint8_t blank[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
    static const uint8_t dot[5] = {0x00, 0x60, 0x60, 0x00, 0x00};
    static const uint8_t colon[5] = {0x00, 0x36, 0x36, 0x00, 0x00};
    static const uint8_t dash[5] = {0x08, 0x08, 0x08, 0x08, 0x08};
    static const uint8_t slash[5] = {0x20, 0x10, 0x08, 0x04, 0x02};
    static const uint8_t percent[5] = {0x62, 0x64, 0x08, 0x13, 0x23};
    static const uint8_t greater[5] = {0x08, 0x14, 0x22, 0x41, 0x00};

    static const uint8_t digits[10][5] = {
        {0x3E, 0x51, 0x49, 0x45, 0x3E},
        {0x00, 0x42, 0x7F, 0x40, 0x00},
        {0x42, 0x61, 0x51, 0x49, 0x46},
        {0x21, 0x41, 0x45, 0x4B, 0x31},
        {0x18, 0x14, 0x12, 0x7F, 0x10},
        {0x27, 0x45, 0x45, 0x45, 0x39},
        {0x3C, 0x4A, 0x49, 0x49, 0x30},
        {0x01, 0x71, 0x09, 0x05, 0x03},
        {0x36, 0x49, 0x49, 0x49, 0x36},
        {0x06, 0x49, 0x49, 0x29, 0x1E},
    };

    static const uint8_t letters[26][5] = {
        {0x7E, 0x11, 0x11, 0x11, 0x7E},
        {0x7F, 0x49, 0x49, 0x49, 0x36},
        {0x3E, 0x41, 0x41, 0x41, 0x22},
        {0x7F, 0x41, 0x41, 0x22, 0x1C},
        {0x7F, 0x49, 0x49, 0x49, 0x41},
        {0x7F, 0x09, 0x09, 0x09, 0x01},
        {0x3E, 0x41, 0x49, 0x49, 0x7A},
        {0x7F, 0x08, 0x08, 0x08, 0x7F},
        {0x00, 0x41, 0x7F, 0x41, 0x00},
        {0x20, 0x40, 0x41, 0x3F, 0x01},
        {0x7F, 0x08, 0x14, 0x22, 0x41},
        {0x7F, 0x40, 0x40, 0x40, 0x40},
        {0x7F, 0x02, 0x0C, 0x02, 0x7F},
        {0x7F, 0x04, 0x08, 0x10, 0x7F},
        {0x3E, 0x41, 0x41, 0x41, 0x3E},
        {0x7F, 0x09, 0x09, 0x09, 0x06},
        {0x3E, 0x41, 0x51, 0x21, 0x5E},
        {0x7F, 0x09, 0x19, 0x29, 0x46},
        {0x46, 0x49, 0x49, 0x49, 0x31},
        {0x01, 0x01, 0x7F, 0x01, 0x01},
        {0x3F, 0x40, 0x40, 0x40, 0x3F},
        {0x1F, 0x20, 0x40, 0x20, 0x1F},
        {0x7F, 0x20, 0x18, 0x20, 0x7F},
        {0x63, 0x14, 0x08, 0x14, 0x63},
        {0x03, 0x04, 0x78, 0x04, 0x03},
        {0x61, 0x51, 0x49, 0x45, 0x43},
    };

    if (c >= '0' && c <= '9') {
        return digits[c - '0'];
    }
    if (c >= 'A' && c <= 'Z') {
        return letters[c - 'A'];
    }

    switch (c) {
    case '.':
        return dot;
    case ':':
        return colon;
    case '-':
        return dash;
    case '/':
        return slash;
    case '%':
        return percent;
    case '>':
        return greater;
    case ' ':
    default:
        return blank;
    }
}

static void framebuffer_clear(void)
{
    memset(s_framebuffer, 0, sizeof(s_framebuffer));
}

static void draw_char(uint8_t line, uint8_t column, char c)
{
    if (line >= 8 || column >= 21) {
        return;
    }

    const uint8_t *glyph = glyph_for_char(c);
    uint16_t offset = (uint16_t)line * APP_OLED_WIDTH + (uint16_t)column * 6;
    for (int i = 0; i < 5; ++i) {
        s_framebuffer[offset + i] = glyph[i];
    }
    s_framebuffer[offset + 5] = 0x00;
}

static void draw_text(uint8_t line, const char *text)
{
    for (uint8_t i = 0; i < 21; ++i) {
        draw_char(line, i, (text != NULL && text[i] != '\0') ? text[i] : ' ');
    }
}

static esp_err_t flush_framebuffer(void)
{
    for (uint8_t page = 0; page < 8; ++page) {
        uint8_t page_setup[] = {0x00, (uint8_t)(0xB0 + page), 0x00, 0x10};
        ESP_RETURN_ON_ERROR(
            i2c_master_write_to_device(APP_OLED_I2C_PORT, APP_OLED_I2C_ADDRESS, page_setup, sizeof(page_setup), pdMS_TO_TICKS(100)),
            TAG,
            "oled page setup failed");

        uint8_t page_payload[APP_OLED_WIDTH + 1];
        page_payload[0] = 0x40;
        memcpy(&page_payload[1], &s_framebuffer[page * APP_OLED_WIDTH], APP_OLED_WIDTH);
        ESP_RETURN_ON_ERROR(
            i2c_master_write_to_device(APP_OLED_I2C_PORT, APP_OLED_I2C_ADDRESS, page_payload, sizeof(page_payload), pdMS_TO_TICKS(100)),
            TAG,
            "oled page write failed");
    }

    return ESP_OK;
}

static void render_limit_line(char *buffer, size_t buffer_size, const ui_state_t *state, limit_field_t field)
{
    char value[12];
    alarm_service_limit_value_to_string(&state->limits, field, value, sizeof(value));
    snprintf(buffer, buffer_size, "%c%s:%s",
        state->selected_limit == field ? '>' : ' ',
        alarm_service_limit_name(field),
        value);
}

esp_err_t display_service_init(void)
{
    i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = APP_OLED_SDA_PIN,
        .scl_io_num = APP_OLED_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };

    ESP_ERROR_CHECK(i2c_param_config(APP_OLED_I2C_PORT, &i2c_cfg));
    ESP_ERROR_CHECK(i2c_driver_install(APP_OLED_I2C_PORT, I2C_MODE_MASTER, 0, 0, 0));

    const uint8_t init_sequence[] = {
        0xAE, 0x20, 0x00, 0x40, 0xA1, 0xC8, 0x81, 0x7F,
        0xA6, 0xA8, 0x3F, 0xD3, 0x00, 0xD5, 0x80, 0xD9,
        0xF1, 0xDA, 0x12, 0xDB, 0x40, 0x8D, 0x14, 0xAF,
    };

    for (size_t i = 0; i < sizeof(init_sequence); ++i) {
        ESP_RETURN_ON_ERROR(oled_command(init_sequence[i]), TAG, "oled init failed");
    }

    framebuffer_clear();
    return flush_framebuffer();
}

esp_err_t display_service_render(const ui_state_t *state)
{
    if (state == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    char lines[8][22];
    char alarm_text[APP_MAX_ALARM_TEXT];
    int start_index = 0;

    if (state->selected_limit > 2) {
        start_index = state->selected_limit - 2;
    }
    if (start_index > (LIMIT_FIELD_COUNT - 4)) {
        start_index = LIMIT_FIELD_COUNT - 4;
    }

    alarm_service_describe(state->active_alarms, alarm_text, sizeof(alarm_text));

    snprintf(lines[0], sizeof(lines[0]), "W:%s M:%s T:%s",
        state->wifi_connected ? "OK" : "NO",
        state->mqtt_connected ? "OK" : "NO",
        state->time_synced ? "OK" : "NO");

    if (state->sample.dht_valid) {
        snprintf(lines[1], sizeof(lines[1]), "T:%.1fC H:%.1f%%", state->sample.temperature_c, state->sample.humidity_pct);
    } else {
        snprintf(lines[1], sizeof(lines[1]), "DHT SENSOR FAIL");
    }

    snprintf(lines[2], sizeof(lines[2]), "L:%d B:%s", state->sample.luminosity_raw, state->buzzer_active ? "ON" : "OFF");
    snprintf(lines[3], sizeof(lines[3]), "AL:%.18s", alarm_text);

    for (int i = 0; i < 4; ++i) {
        render_limit_line(lines[4 + i], sizeof(lines[4 + i]), state, (limit_field_t)(start_index + i));
    }

    framebuffer_clear();
    for (int i = 0; i < 8; ++i) {
        draw_text(i, lines[i]);
    }

    return flush_framebuffer();
}
