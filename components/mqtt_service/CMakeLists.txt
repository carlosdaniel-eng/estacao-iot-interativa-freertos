idf_component_register(
    SRCS "src/mqtt_service.c"
    INCLUDE_DIRS "include"
    REQUIRES mqtt app_common app_config alarm_service
)
