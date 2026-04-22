idf_component_register(
    SRCS "src/keypad_service.c"
    INCLUDE_DIRS "include"
    REQUIRES app_config driver freertos
)
