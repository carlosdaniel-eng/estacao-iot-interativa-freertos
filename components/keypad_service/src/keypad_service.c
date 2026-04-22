#include "keypad_service.h"

#include <stdbool.h>
#include "freertos/task.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "app_config.h"

static QueueHandle_t s_key_queue;

static const char KEYMAP[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
};

static char scan_keypad(void)
{
    for (int row = 0; row < 4; ++row) {
        for (int i = 0; i < 4; ++i) {
            gpio_set_level(APP_KEYPAD_ROWS[i], 1);
        }
        gpio_set_level(APP_KEYPAD_ROWS[row], 0);
        ets_delay_us(80);

        for (int col = 0; col < 4; ++col) {
            if (gpio_get_level(APP_KEYPAD_COLS[col]) == 0) {
                return KEYMAP[row][col];
            }
        }
    }

    return '\0';
}

static void keypad_task(void *arg)
{
    (void)arg;
    char last_key = '\0';

    while (true) {
        char key = scan_keypad();
        if (key != '\0' && key != last_key) {
            xQueueSend(s_key_queue, &key, 0);
        }

        last_key = key;
        vTaskDelay(pdMS_TO_TICKS(APP_KEYPAD_PERIOD_MS));
    }
}

esp_err_t keypad_service_start(QueueHandle_t key_queue)
{
    s_key_queue = key_queue;

    for (int i = 0; i < 4; ++i) {
        gpio_config_t row_cfg = {
            .pin_bit_mask = 1ULL << APP_KEYPAD_ROWS[i],
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ESP_ERROR_CHECK(gpio_config(&row_cfg));
        gpio_set_level(APP_KEYPAD_ROWS[i], 1);

        gpio_config_t col_cfg = {
            .pin_bit_mask = 1ULL << APP_KEYPAD_COLS[i],
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ESP_ERROR_CHECK(gpio_config(&col_cfg));
    }

    xTaskCreate(keypad_task, "keypad_task", 3072, NULL, 4, NULL);
    return ESP_OK;
}
