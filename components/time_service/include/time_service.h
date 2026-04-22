#pragma once

#include <stdbool.h>
#include <stddef.h>

void time_service_start(void);
bool time_service_is_synced(void);
void time_service_format_now(char *buffer, size_t buffer_size);
