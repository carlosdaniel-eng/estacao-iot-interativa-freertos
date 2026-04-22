#include "mqtt_service.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "mqtt_client.h"
#include "app_config.h"
#include "alarm_service.h"

static const char *TAG = "mqtt_service";

static esp_mqtt_client_handle_t s_client;
static mqtt_rgb_callback_t s_rgb_callback;
static bool s_connected;
static rgb_led_state_t s_led_state;
static char s_topic_temperature[64];
static char s_topic_humidity[64];
static char s_topic_luminosity[64];
static char s_topic_alarm[64];
static char s_topic_led_red[64];
static char s_topic_led_green[64];
static char s_topic_led_blue[64];

static bool topic_matches(esp_mqtt_event_handle_t event, const char *topic)
{
    size_t length = strlen(topic);
    return event->topic_len == (int)length && strncmp(event->topic, topic, length) == 0;
}

static int parse_int_payload(const char *data, int data_len)
{
    char buffer[16];
    size_t copy_len = data_len < (int)(sizeof(buffer) - 1) ? (size_t)data_len : sizeof(buffer) - 1;
    memcpy(buffer, data, copy_len);
    buffer[copy_len] = '\0';
    return atoi(buffer);
}

static uint8_t clamp_u8(int value)
{
    if (value < 0) {
        return 0;
    }
    if (value > 255) {
        return 255;
    }
    return (uint8_t)value;
}

static void handle_led_message(esp_mqtt_event_handle_t event)
{
    if (topic_matches(event, s_topic_led_red)) {
        s_led_state.red = clamp_u8(parse_int_payload(event->data, event->data_len));
    } else if (topic_matches(event, s_topic_led_green)) {
        s_led_state.green = clamp_u8(parse_int_payload(event->data, event->data_len));
    } else if (topic_matches(event, s_topic_led_blue)) {
        s_led_state.blue = clamp_u8(parse_int_payload(event->data, event->data_len));
    } else {
        return;
    }

    if (s_rgb_callback != NULL) {
        s_rgb_callback(&s_led_state);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    (void)handler_args;
    (void)base;
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        s_connected = true;
        esp_mqtt_client_subscribe(s_client, s_topic_led_red, 1);
        esp_mqtt_client_subscribe(s_client, s_topic_led_green, 1);
        esp_mqtt_client_subscribe(s_client, s_topic_led_blue, 1);
        ESP_LOGI(TAG, "MQTT connected");
        break;
    case MQTT_EVENT_DISCONNECTED:
        s_connected = false;
        ESP_LOGW(TAG, "MQTT disconnected");
        break;
    case MQTT_EVENT_DATA:
        handle_led_message(event);
        break;
    default:
        break;
    }
}

esp_err_t mqtt_service_start(mqtt_rgb_callback_t callback)
{
    s_rgb_callback = callback;
    s_led_state = (rgb_led_state_t){0};

    app_config_build_feed_topic("temperatura", s_topic_temperature, sizeof(s_topic_temperature));
    app_config_build_feed_topic("umidade", s_topic_humidity, sizeof(s_topic_humidity));
    app_config_build_feed_topic("luminosidade", s_topic_luminosity, sizeof(s_topic_luminosity));
    app_config_build_feed_topic("alarmes", s_topic_alarm, sizeof(s_topic_alarm));
    app_config_build_feed_topic("led-red", s_topic_led_red, sizeof(s_topic_led_red));
    app_config_build_feed_topic("led-green", s_topic_led_green, sizeof(s_topic_led_green));
    app_config_build_feed_topic("led-blue", s_topic_led_blue, sizeof(s_topic_led_blue));

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = APP_MQTT_BROKER_URI,
        .credentials.username = APP_ADAFRUIT_IO_USERNAME,
        .credentials.authentication.password = APP_ADAFRUIT_IO_KEY,
    };

    s_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_client);
    return ESP_OK;
}

bool mqtt_service_is_connected(void)
{
    return s_connected;
}

esp_err_t mqtt_service_publish_sample(const sensor_sample_t *sample)
{
    if (!s_connected || sample == NULL) {
        return ESP_FAIL;
    }

    char payload[24];
    snprintf(payload, sizeof(payload), "%.1f", sample->temperature_c);
    esp_mqtt_client_publish(s_client, s_topic_temperature, payload, 0, 1, 0);

    snprintf(payload, sizeof(payload), "%.1f", sample->humidity_pct);
    esp_mqtt_client_publish(s_client, s_topic_humidity, payload, 0, 1, 0);

    snprintf(payload, sizeof(payload), "%d", sample->luminosity_raw);
    esp_mqtt_client_publish(s_client, s_topic_luminosity, payload, 0, 1, 0);

    return ESP_OK;
}

esp_err_t mqtt_service_publish_alarm(alarm_flags_t flags)
{
    if (!s_connected) {
        return ESP_FAIL;
    }

    char payload[APP_MAX_ALARM_TEXT];
    alarm_service_describe(flags, payload, sizeof(payload));
    esp_mqtt_client_publish(s_client, s_topic_alarm, payload, 0, 1, 0);
    return ESP_OK;
}
