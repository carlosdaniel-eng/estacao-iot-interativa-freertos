#include "storage_service.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_check.h"
#include "esp_log.h"
#include "nvs.h"
#include "app_config.h"
#include "alarm_service.h"

static const char *TAG = "storage_service";

static esp_err_t open_storage(nvs_handle_t *handle)
{
    return nvs_open(APP_NVS_NAMESPACE, NVS_READWRITE, handle);
}

esp_err_t storage_service_init(void)
{
    ESP_LOGI(TAG, "NVS storage ready");
    return ESP_OK;
}

esp_err_t storage_service_load_limits(alarm_limits_t *limits)
{
    if (limits == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle;
    esp_err_t err = open_storage(&handle);
    if (err != ESP_OK) {
        return err;
    }

    size_t size = sizeof(*limits);
    err = nvs_get_blob(handle, APP_NVS_LIMITS_KEY, limits, &size);
    nvs_close(handle);
    return err;
}

esp_err_t storage_service_save_limits(const alarm_limits_t *limits)
{
    if (limits == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle;
    esp_err_t err = open_storage(&handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_blob(handle, APP_NVS_LIMITS_KEY, limits, sizeof(*limits));
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err;
}

esp_err_t storage_service_append_alarm(const alarm_log_entry_t *entry)
{
    if (entry == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    char flags_text[APP_MAX_ALARM_TEXT];
    alarm_service_describe(entry->flags, flags_text, sizeof(flags_text));

    char line[192];
    snprintf(
        line,
        sizeof(line),
        "%s - T:%.1fC H:%.1f%% L:%d FLAGS:%s\n",
        entry->timestamp,
        entry->sample.temperature_c,
        entry->sample.humidity_pct,
        entry->sample.luminosity_raw,
        flags_text);

    esp_err_t result = ESP_OK;
    nvs_handle_t handle;
    esp_err_t err = open_storage(&handle);
    if (err != ESP_OK) {
        return err;
    }

    size_t current_size = 0;
    err = nvs_get_str(handle, APP_NVS_LOG_KEY, NULL, &current_size);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        current_size = 0;
    } else if (err != ESP_OK) {
        nvs_close(handle);
        return err;
    }

    char *current = calloc(current_size == 0 ? 1 : current_size, sizeof(char));
    if (current == NULL) {
        nvs_close(handle);
        return ESP_ERR_NO_MEM;
    }

    if (current_size > 0) {
        result = nvs_get_str(handle, APP_NVS_LOG_KEY, current, &current_size);
        if (result != ESP_OK) {
            goto cleanup;
        }
    } else {
        current[0] = '\0';
    }

    size_t merged_size = strlen(current) + strlen(line) + 1;
    char *merged = calloc(merged_size, sizeof(char));
    if (merged == NULL) {
        free(current);
        nvs_close(handle);
        return ESP_ERR_NO_MEM;
    }

    snprintf(merged, merged_size, "%s%s", current, line);

    if (strlen(merged) >= APP_LOG_STORAGE_BYTES) {
        size_t offset = strlen(merged) - (APP_LOG_STORAGE_BYTES - 1);
        memmove(merged, merged + offset, strlen(merged + offset) + 1);
    }

    result = nvs_set_str(handle, APP_NVS_LOG_KEY, merged);
    if (result == ESP_OK) {
        result = nvs_commit(handle);
    }

cleanup_merged:
    free(merged);
cleanup:
    free(current);
    nvs_close(handle);
    return result;
}

void storage_service_print_alarm_logs(void)
{
    nvs_handle_t handle;
    if (open_storage(&handle) != ESP_OK) {
        printf("ALARM LOGS: unavailable\n");
        return;
    }

    size_t size = 0;
    esp_err_t err = nvs_get_str(handle, APP_NVS_LOG_KEY, NULL, &size);
    if (err == ESP_ERR_NVS_NOT_FOUND || size == 0) {
        printf("ALARM LOGS:\n<empty>\n");
        nvs_close(handle);
        return;
    }

    char *buffer = calloc(size, sizeof(char));
    if (buffer == NULL) {
        printf("ALARM LOGS: out of memory\n");
        nvs_close(handle);
        return;
    }

    if (nvs_get_str(handle, APP_NVS_LOG_KEY, buffer, &size) == ESP_OK) {
        printf("ALARM LOGS:\n%s", buffer);
    }

    free(buffer);
    nvs_close(handle);
}

esp_err_t storage_service_clear_alarm_logs(void)
{
    nvs_handle_t handle;
    ESP_RETURN_ON_ERROR(open_storage(&handle), TAG, "nvs open failed");
    esp_err_t err = nvs_erase_key(handle, APP_NVS_LOG_KEY);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        nvs_close(handle);
        return err;
    }

    ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "commit clear logs failed");
    nvs_close(handle);
    return ESP_OK;
}
