idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES sensor haptic rtsa_transport freertos esp_timer
)
