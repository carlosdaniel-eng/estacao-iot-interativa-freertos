idf_component_register(
    SRCS "app_main.c"
    INCLUDE_DIRS "."
    REQUIRES
        app_common
        app_config
        wifi_manager
        mqtt_service
        sensor_service
        actuator_service
        display_service
        keypad_service
        storage_service
        alarm_service
        time_service
        console_service
)
