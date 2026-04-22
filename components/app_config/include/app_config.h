#pragma once

#include <stddef.h>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_adc/adc_oneshot.h"
#include "app_types.h"

#define APP_WIFI_SSID "SET_WIFI_SSID"
#define APP_WIFI_PASSWORD "SET_WIFI_PASSWORD"
#define APP_ADAFRUIT_IO_USERNAME "SET_ADAFRUIT_USERNAME"
#define APP_ADAFRUIT_IO_KEY "SET_ADAFRUIT_IO_KEY"
#define APP_MQTT_BROKER_URI "mqtts://io.adafruit.com"

#define APP_SENSOR_PERIOD_MS 5000
#define APP_UI_PERIOD_MS 400
#define APP_KEYPAD_PERIOD_MS 60

#define APP_OLED_WIDTH 128
#define APP_OLED_HEIGHT 64
#define APP_OLED_I2C_PORT I2C_NUM_0
#define APP_OLED_I2C_ADDRESS 0x3C
#define APP_OLED_SDA_PIN GPIO_NUM_6
#define APP_OLED_SCL_PIN GPIO_NUM_7

#define APP_DHT11_PIN GPIO_NUM_2
#define APP_LDR_ADC_UNIT ADC_UNIT_1
#define APP_LDR_ADC_CHANNEL ADC_CHANNEL_0

#define APP_RGB_RED_PIN GPIO_NUM_8
#define APP_RGB_GREEN_PIN GPIO_NUM_9
#define APP_RGB_BLUE_PIN GPIO_NUM_10
#define APP_BUZZER_PIN GPIO_NUM_18

#define APP_BUZZER_FREQUENCY_HZ 2400
#define APP_BUZZER_DUTY 128

#define APP_NVS_NAMESPACE "iot_station"
#define APP_NVS_LIMITS_KEY "limits"
#define APP_NVS_LOG_KEY "alarm_log"
#define APP_LOG_STORAGE_BYTES 4096

extern const gpio_num_t APP_KEYPAD_ROWS[4];
extern const gpio_num_t APP_KEYPAD_COLS[4];

void app_config_get_default_limits(alarm_limits_t *limits);
void app_config_build_feed_topic(const char *feed_name, char *buffer, size_t buffer_size);
