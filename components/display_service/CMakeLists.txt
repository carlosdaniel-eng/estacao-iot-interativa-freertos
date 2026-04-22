idf_component_register(
    SRCS "src/display_service.c"
    INCLUDE_DIRS "include"
    REQUIRES app_common app_config driver
)
