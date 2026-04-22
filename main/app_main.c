#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "app_config.h"
#include "app_types.h"
#include "wifi_manager.h"
#include "mqtt_service.h"
#include "sensor_service.h"
#include "actuator_service.h"
#include "display_service.h"
#include "keypad_service.h"
#include "storage_service.h"
#include "alarm_service.h"
#include "time_service.h"
#include "console_service.h"

static const char *TAG = "app_main";

static ui_state_t s_ui_state;
static SemaphoreHandle_t s_state_mutex;
static QueueHandle_t s_key_queue;
static bool s_alarm_episode_active;

static void set_status_locked(const char *status)
{
    snprintf(s_ui_state.status_text, sizeof(s_ui_state.status_text), "%s", status);
}

static void mqtt_rgb_callback(const rgb_led_state_t *state)
{
    if (state == NULL) {
        return;
    }

    actuator_service_set_rgb(state);

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE) {
        s_ui_state.led_state = *state;
        set_status_locked("LED VIA MQTT");
        xSemaphoreGive(s_state_mutex);
    }
}

static void save_limits_locked(void)
{
    storage_service_save_limits(&s_ui_state.limits);
}

static void handle_key(char key)
{
    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) != pdTRUE) {
        return;
    }

    switch (key) {
    case 'A':
        s_ui_state.selected_limit = (s_ui_state.selected_limit + LIMIT_FIELD_COUNT - 1) % LIMIT_FIELD_COUNT;
        set_status_locked("FIELD PREV");
        break;
    case 'B':
        s_ui_state.selected_limit = (s_ui_state.selected_limit + 1) % LIMIT_FIELD_COUNT;
        set_status_locked("FIELD NEXT");
        break;
    case 'C':
        alarm_service_adjust_limit(&s_ui_state.limits, (limit_field_t)s_ui_state.selected_limit, -1);
        save_limits_locked();
        set_status_locked("LIMIT DOWN");
        break;
    case 'D':
        alarm_service_adjust_limit(&s_ui_state.limits, (limit_field_t)s_ui_state.selected_limit, 1);
        save_limits_locked();
        set_status_locked("LIMIT UP");
        break;
    case '*':
        actuator_service_set_buzzer(false);
        s_ui_state.buzzer_active = false;
        set_status_locked("BUZZER OFF");
        break;
    case '#':
        save_limits_locked();
        set_status_locked("LIMITS SAVED");
        break;
    default:
        break;
    }

    xSemaphoreGive(s_state_mutex);
}

static void input_task(void *arg)
{
    (void)arg;
    char key = '\0';

    while (true) {
        if (xQueueReceive(s_key_queue, &key, portMAX_DELAY) == pdTRUE) {
            handle_key(key);
        }
    }
}

static void ui_task(void *arg)
{
    (void)arg;
    ui_state_t snapshot;

    while (true) {
        if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE) {
            snapshot = s_ui_state;
            xSemaphoreGive(s_state_mutex);
            display_service_render(&snapshot);
        }

        vTaskDelay(pdMS_TO_TICKS(APP_UI_PERIOD_MS));
    }
}

static void sensor_task(void *arg)
{
    (void)arg;

    while (true) {
        sensor_sample_t sample;
        alarm_limits_t limits = {0};
        alarm_flags_t flags = ALARM_FLAG_SENSOR_FAULT;
        bool new_alarm = false;

        if (sensor_service_read(&sample) == ESP_OK) {
            if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE) {
                limits = s_ui_state.limits;
                xSemaphoreGive(s_state_mutex);
            }

            flags = alarm_service_evaluate(&sample, &limits);

            if (flags == ALARM_FLAG_NONE) {
                s_alarm_episode_active = false;
            } else if (!s_alarm_episode_active) {
                char timestamp[24];
                alarm_log_entry_t entry = {
                    .sample = sample,
                    .flags = flags,
                };

                time_service_format_now(timestamp, sizeof(timestamp));
                snprintf(entry.timestamp, sizeof(entry.timestamp), "%s", timestamp);

                storage_service_append_alarm(&entry);
                actuator_service_set_buzzer(true);
                s_alarm_episode_active = true;
                new_alarm = true;
            }

            mqtt_service_publish_sample(&sample);
            mqtt_service_publish_alarm(flags);

            if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE) {
                s_ui_state.sample = sample;
                s_ui_state.active_alarms = flags;
                s_ui_state.wifi_connected = wifi_manager_is_connected();
                s_ui_state.mqtt_connected = mqtt_service_is_connected();
                s_ui_state.time_synced = time_service_is_synced();
                s_ui_state.buzzer_active = actuator_service_is_buzzer_active();
                set_status_locked(new_alarm ? "ALARM LATCHED" : "MONITORING");
                xSemaphoreGive(s_state_mutex);
            }
        } else {
            ESP_LOGW(TAG, "Sensor read failed");
        }

        vTaskDelay(pdMS_TO_TICKS(APP_SENSOR_PERIOD_MS));
    }
}

void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    s_state_mutex = xSemaphoreCreateMutex();
    s_key_queue = xQueueCreate(8, sizeof(char));

    app_config_get_default_limits(&s_ui_state.limits);
    s_ui_state.selected_limit = LIMIT_FIELD_TEMP_MIN;
    set_status_locked("BOOT");

    ESP_ERROR_CHECK(storage_service_init());
    if (storage_service_load_limits(&s_ui_state.limits) != ESP_OK) {
        storage_service_save_limits(&s_ui_state.limits);
    }

    ESP_ERROR_CHECK(sensor_service_init());
    ESP_ERROR_CHECK(actuator_service_init());
    ESP_ERROR_CHECK(display_service_init());
    ESP_ERROR_CHECK(wifi_manager_start());
    time_service_start();
    ESP_ERROR_CHECK(mqtt_service_start(mqtt_rgb_callback));
    ESP_ERROR_CHECK(keypad_service_start(s_key_queue));
    ESP_ERROR_CHECK(console_service_start(&s_ui_state.limits, s_state_mutex));

    xTaskCreate(sensor_task, "sensor_task", 6144, NULL, 5, NULL);
    xTaskCreate(ui_task, "ui_task", 6144, NULL, 4, NULL);
    xTaskCreate(input_task, "input_task", 4096, NULL, 4, NULL);
}
