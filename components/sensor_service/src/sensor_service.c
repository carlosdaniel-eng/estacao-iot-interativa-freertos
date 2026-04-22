#include "sensor_service.h"

#include <string.h>
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "app_config.h"

static const char *TAG = "sensor_service";
static adc_oneshot_unit_handle_t s_adc_handle;

static int wait_for_level(int level, uint32_t timeout_us)
{
    int64_t start = esp_timer_get_time();
    while (gpio_get_level(APP_DHT11_PIN) == level) {
        if ((esp_timer_get_time() - start) > timeout_us) {
            return -1;
        }
    }
    return (int)(esp_timer_get_time() - start);
}

static esp_err_t read_dht11(float *temperature_c, float *humidity_pct)
{
    uint8_t data[5] = {0};

    gpio_set_direction(APP_DHT11_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(APP_DHT11_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(APP_DHT11_PIN, 1);
    esp_rom_delay_us(40);
    gpio_set_direction(APP_DHT11_PIN, GPIO_MODE_INPUT);

    if (wait_for_level(1, 100) < 0 || wait_for_level(0, 100) < 0 || wait_for_level(1, 100) < 0) {
        return ESP_FAIL;
    }

    for (int bit = 0; bit < 40; ++bit) {
        if (wait_for_level(0, 70) < 0) {
            return ESP_FAIL;
        }

        int high_time = wait_for_level(1, 100);
        if (high_time < 0) {
            return ESP_FAIL;
        }

        data[bit / 8] <<= 1;
        if (high_time > 40) {
            data[bit / 8] |= 1;
        }
    }

    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        return ESP_ERR_INVALID_CRC;
    }

    *humidity_pct = (float)data[0];
    *temperature_c = (float)data[2];
    return ESP_OK;
}

esp_err_t sensor_service_init(void)
{
    adc_oneshot_unit_init_cfg_t adc_config = {
        .unit_id = APP_LDR_ADC_UNIT,
    };
    adc_oneshot_chan_cfg_t ldr_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_RETURN_ON_ERROR(adc_oneshot_new_unit(&adc_config, &s_adc_handle), TAG, "adc init failed");
    ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(s_adc_handle, APP_LDR_ADC_CHANNEL, &ldr_config), TAG, "adc channel failed");

    gpio_config_t dht_gpio_config = {
        .pin_bit_mask = 1ULL << APP_DHT11_PIN,
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&dht_gpio_config), TAG, "dht gpio failed");

    return ESP_OK;
}

esp_err_t sensor_service_read(sensor_sample_t *sample)
{
    if (sample == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(sample, 0, sizeof(*sample));

    int ldr_value = 0;
    ESP_RETURN_ON_ERROR(adc_oneshot_read(s_adc_handle, APP_LDR_ADC_CHANNEL, &ldr_value), TAG, "adc read failed");

    sample->luminosity_raw = ldr_value;
    sample->timestamp_ms = esp_timer_get_time() / 1000;

    if (read_dht11(&sample->temperature_c, &sample->humidity_pct) == ESP_OK) {
        sample->dht_valid = true;
    } else {
        sample->dht_valid = false;
        sample->temperature_c = 0.0f;
        sample->humidity_pct = 0.0f;
        ESP_LOGW(TAG, "DHT11 read failed");
    }

    return ESP_OK;
}
