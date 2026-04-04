# cmake/rtsa_sdk.cmake
# Downloads and extracts the Agora RTSA SDK (pre-built for Xtensa LX7 / ESP32-S3).
# Called from the top-level CMakeLists.txt BEFORE the ESP-IDF project() include so
# that RTSA_SDK_* cache variables are visible to component CMakeLists files.
#
# After a successful configure the following cache variables are set:
#   RTSA_SDK_DIR         — root of the extracted SDK  (agora_sdk/)
#   RTSA_SDK_INCLUDE_DIR — header directory            (agora_sdk/include/)
#   RTSA_SDK_LIB         — static library path         (agora_sdk/lib/xtensa_lx7/libagora-rtc-sdk.a)

include(FetchContent)

set(RTSA_SDK_URL
    "https://download.agora.io/rtsasdk/release/Agora-RTSALite-SRmRcAcAj-xtensa_lx7-freertos-gnu-551-esp32s3-v1.10.0-20260404_134349-1111.tgz"
    CACHE STRING "URL of the Agora RTSA SDK tarball" FORCE
)

FetchContent_Declare(
    rtsa_sdk
    URL                    ${RTSA_SDK_URL}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Use FetchContent_Populate (not MakeAvailable) — the SDK has no CMakeLists.txt
# so we only want to download/extract, not configure/build.
FetchContent_GetProperties(rtsa_sdk)
if(NOT rtsa_sdk_POPULATED)
    message(STATUS "Downloading Agora RTSA SDK...")
    FetchContent_Populate(rtsa_sdk)
    message(STATUS "Agora RTSA SDK extracted to: ${rtsa_sdk_SOURCE_DIR}")
endif()

# FetchContent strips the top-level tarball directory (agora_rtsa_sdk/) so the
# SDK root lands directly at rtsa_sdk_SOURCE_DIR/agora_sdk/.
set(RTSA_SDK_DIR         "${rtsa_sdk_SOURCE_DIR}/agora_sdk"
    CACHE PATH     "Agora RTSA SDK root directory" FORCE)
set(RTSA_SDK_INCLUDE_DIR "${RTSA_SDK_DIR}/include"
    CACHE PATH     "Agora RTSA SDK include directory" FORCE)
set(RTSA_SDK_LIB         "${RTSA_SDK_DIR}/lib/xtensa_lx7/libagora-rtc-sdk.a"
    CACHE FILEPATH "Agora RTSA SDK static library" FORCE)

message(STATUS "RTSA_SDK_DIR:         ${RTSA_SDK_DIR}")
message(STATUS "RTSA_SDK_INCLUDE_DIR: ${RTSA_SDK_INCLUDE_DIR}")
message(STATUS "RTSA_SDK_LIB:         ${RTSA_SDK_LIB}")
