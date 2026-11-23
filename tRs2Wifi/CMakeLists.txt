# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

if(NOT DEFINED SDKCONFIG OR SDKCONFIG STREQUAL "${CMAKE_SOURCE_DIR}/sdkconfig")
    set(SDKCONFIG "${CMAKE_SOURCE_DIR}/sdkconfig.local")
endif()

if(NOT EXISTS "${SDKCONFIG}" AND EXISTS "${CMAKE_SOURCE_DIR}/sdkconfig")
    configure_file("${CMAKE_SOURCE_DIR}/sdkconfig" "${SDKCONFIG}" COPYONLY)
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(app-template)
