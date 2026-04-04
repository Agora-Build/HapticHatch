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
    REQUIRES log freertos
)

# Link the pre-built Agora RTSA SDK static library when it has been downloaded.
if(DEFINED RTSA_SDK_LIB AND EXISTS "${RTSA_SDK_LIB}")
    target_link_libraries(${COMPONENT_LIB} PRIVATE "${RTSA_SDK_LIB}")
    message(STATUS "rtsa_transport: linking ${RTSA_SDK_LIB}")
endif()
