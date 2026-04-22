idf_component_register(
    SRCS "src/actuator_service.c"
    INCLUDE_DIRS "include"
    REQUIRES app_common app_config driver
)
