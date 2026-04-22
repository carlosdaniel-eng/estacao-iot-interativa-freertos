idf_component_register(
    SRCS "src/console_service.c"
    INCLUDE_DIRS "include"
    REQUIRES app_common freertos storage_service
)
