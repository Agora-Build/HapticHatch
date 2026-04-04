idf_component_register(
    SRCS "rtsa_transport.c"
    INCLUDE_DIRS "include" "${RTSA_SDK_INCLUDE_DIR}"
    REQUIRES log
)

# Link the pre-built Agora RTSA SDK static library when it has been downloaded.
# RTSA_SDK_LIB is set by cmake/rtsa_sdk.cmake during the configure phase.
if(DEFINED RTSA_SDK_LIB AND EXISTS "${RTSA_SDK_LIB}")
    target_link_libraries(${COMPONENT_LIB} PRIVATE "${RTSA_SDK_LIB}")
    message(STATUS "rtsa_transport: linking ${RTSA_SDK_LIB}")
endif()
