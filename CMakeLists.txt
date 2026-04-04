cmake_minimum_required(VERSION 3.16)
set(EXTRA_COMPONENT_DIRS components)

# Download Agora RTSA SDK before ESP-IDF configure so RTSA_SDK_* vars are
# available to component CMakeLists files during the IDF build graph walk.
include(cmake/rtsa_sdk.cmake)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(HapticHatch)
