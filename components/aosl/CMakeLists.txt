# AOSL (Agora OS Abstraction Layer) ESP-IDF component
# Source tree lives at ../../aosl relative to this project root,
# i.e. three levels up from this file: components/aosl/ -> ../../../aosl
set(AOSL_DIR "${COMPONENT_PATH}/../../../aosl")
set(CONFIG_PLATFORM "esp32-s3")
set(AOSL_DECLARE_PROJECT OFF CACHE BOOL "Declare as Standalone Project" FORCE)

include(${AOSL_DIR}/CMakeLists.txt)

idf_component_register(
    SRCS             ${AOSL_ADD_SOURCES}
    INCLUDE_DIRS     ${AOSL_ADD_INCLUDES_PUBLIC}
    PRIV_INCLUDE_DIRS ${AOSL_ADD_INCLUDES_PRIVATE}
    REQUIRES         spi_flash newlib lwip mbedtls freertos esp_netif
)

target_compile_options(${COMPONENT_LIB} PRIVATE
    ${AOSL_ADD_OPTIONS}
    ${AOSL_ADD_DEFINITIONS}
    -include api/aosl_log.h   # workaround: aosl_hal_iomp.c uses AOSL_LOG_ERR without including it
)
