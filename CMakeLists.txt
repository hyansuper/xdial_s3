cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-incompatible-pointer-types")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-int-conversion")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS ./components/my_esp_util ./components/esp_fetch_online_data)



set(PROJ_LANG "ZH")
set(PROJ_REGION "CN")



add_compile_definitions(PROJ_NAME="${CMAKE_PROJECT_NAME}" 
                        PROJ_LANG="${PROJ_LANG}" PROJ_LANG_${PROJ_LANG}
                        PROJ_REGION="${PROJ_REGION}" PROJ_REGION_${PROJ_REGION})


project(xDial)
