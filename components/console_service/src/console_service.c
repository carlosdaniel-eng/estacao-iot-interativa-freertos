#include "console_service.h"

#include <stdio.h>
#include <string.h>
#include "freertos/task.h"
#include "storage_service.h"

typedef struct {
    const alarm_limits_t *limits;
    SemaphoreHandle_t limits_mutex;
} console_context_t;

static console_context_t s_context;

static void print_limits(void)
{
    alarm_limits_t snapshot;

    if (xSemaphoreTake(s_context.limits_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        snapshot = *s_context.limits;
        xSemaphoreGive(s_context.limits_mutex);
    } else {
        printf("LIMITS unavailable\n");
        return;
    }

    printf(
        "LIMITS:\n"
        "TEMP MIN: %.1fC\n"
        "TEMP MAX: %.1fC\n"
        "LUX MIN: %d\n"
        "LUX MAX: %d\n"
        "HUM MIN: %.1f%%\n"
        "HUM MAX: %.1f%%\n",
        snapshot.temperature_min,
        snapshot.temperature_max,
        snapshot.luminosity_min,
        snapshot.luminosity_max,
        snapshot.humidity_min,
        snapshot.humidity_max);
}

static void console_task(void *arg)
{
    (void)arg;
    char line[64];

    while (true) {
        if (fgets(line, sizeof(line), stdin) == NULL) {
            clearerr(stdin);
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        line[strcspn(line, "\r\n")] = '\0';

        if (strcmp(line, "logs") == 0) {
            storage_service_print_alarm_logs();
        } else if (strcmp(line, "clearlogs") == 0) {
            storage_service_clear_alarm_logs();
            printf("ALARM LOGS CLEARED\n");
        } else if (strcmp(line, "limits") == 0) {
            print_limits();
        } else if (line[0] != '\0') {
            printf("COMMANDS: logs | clearlogs | limits\n");
        }
    }
}

esp_err_t console_service_start(const alarm_limits_t *limits, SemaphoreHandle_t limits_mutex)
{
    s_context.limits = limits;
    s_context.limits_mutex = limits_mutex;
    xTaskCreate(console_task, "console_task", 4096, NULL, 3, NULL);
    return ESP_OK;
}
