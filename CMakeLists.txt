cmake_minimum_required(VERSION 3.22)
project(aeron_python C)

set(CMAKE_C_STANDARD 17)

# Python 3.8
include_directories(/usr/include/python3.8)

# Aeron Client
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/aeron EXCLUDE_FROM_ALL)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/aeron/aeron-client/src/main/c)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/aeronmodule.c)

install(TARGETS aeron LIBRARY DESTINATION lib)

add_custom_target(build
    COMMAND python3 setup.py build
    COMMAND sudo python3 setup.py install
    DEPENDS ${SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_library(_ EXCLUDE_FROM_ALL ${SOURCES})
