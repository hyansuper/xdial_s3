# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-incompatible-pointer-types")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-int-conversion")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS ./components/my_esp_util ./components/esp_fetch_online_data)


set(APP_REGION "CN")

if(APP_REGION STREQUAL "CN")
    add_compile_definitions(APP_REGION_CN)
else()
    add_compile_definitions(APP_REGION_EN)
endif()

add_compile_definitions(APP_NAME="${CMAKE_PROJECT_NAME}" APP_REGION="${APP_REGION}")



project(xDial)
