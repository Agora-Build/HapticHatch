# Only add the SDK include dir if it exists (guards against download failure)
if(IS_DIRECTORY "${RTSA_SDK_INCLUDE_DIR}")
    set(RTSA_EXTRA_INCLUDES "${RTSA_SDK_INCLUDE_DIR}")
else()
    message(WARNING "RTSA SDK include dir not found: ${RTSA_SDK_INCLUDE_DIR}")
    set(RTSA_EXTRA_INCLUDES "")
endif()

idf_component_register(
    SRCS "rtsa_transport.c"
    INCLUDE_DIRS "include" ${RTSA_EXTRA_INCLUDES}
    REQUIRES log freertos aosl
)

# Link the pre-built Agora RTSA SDK static library when it has been downloaded.
#
# ESP-IDF v5 links components left-to-right (no start-group/end-group).
# libagora-rtc-sdk.a lands in the tail (after all component libs), so it
# cannot pull aosl_* or lwip_* symbols from libraries already scanned.
# Wrapping the tail in its own --start-group/--end-group lets the three
# archives resolve each other without order constraints.
if(DEFINED RTSA_SDK_LIB AND EXISTS "${RTSA_SDK_LIB}")
    target_link_libraries(${COMPONENT_LIB} PRIVATE
        -Wl,--start-group
        "${RTSA_SDK_LIB}"
        "$<TARGET_FILE:__idf_aosl>"
        "$<TARGET_FILE:__idf_lwip>"
        -Wl,--end-group
    )
    message(STATUS "rtsa_transport: linking ${RTSA_SDK_LIB}")
endif()
