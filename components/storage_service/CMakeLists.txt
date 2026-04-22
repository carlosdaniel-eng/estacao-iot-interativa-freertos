idf_component_register(
    SRCS "src/storage_service.c"
    INCLUDE_DIRS "include"
    REQUIRES app_common app_config nvs_flash
)
