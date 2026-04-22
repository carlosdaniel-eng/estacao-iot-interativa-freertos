#include "time_service.h"

#include <stdio.h>
#include <time.h>
#include "esp_sntp.h"

void time_service_start(void)
{
    setenv("TZ", "<-03>3", 1);
    tzset();

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

bool time_service_is_synced(void)
{
    time_t now = 0;
    struct tm tm_info = {0};

    time(&now);
    localtime_r(&now, &tm_info);
    return tm_info.tm_year >= (2024 - 1900);
}

void time_service_format_now(char *buffer, size_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0) {
        return;
    }

    if (!time_service_is_synced()) {
        snprintf(buffer, buffer_size, "UNSYNCED");
        return;
    }

    time_t now = 0;
    struct tm tm_info = {0};
    time(&now);
    localtime_r(&now, &tm_info);
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", &tm_info);
}
