idf_component_register(
    SRCS "main.c" "signaling_demo.c"
    INCLUDE_DIRS "."
    REQUIRES sensor haptic rtsa_transport freertos esp_timer
             esp_wifi esp_netif esp_event nvs_flash
)
