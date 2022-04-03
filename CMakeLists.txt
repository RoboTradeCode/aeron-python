cmake_minimum_required(VERSION 3.21)
project(aeron-python)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_PREFIX .)

include_directories(/usr/include/python3.8)

include(FetchContent)
FetchContent_Declare(aeron GIT_REPOSITORY https://github.com/real-logic/aeron GIT_TAG 1.37.0)
FetchContent_Populate(aeron)
add_subdirectory(${aeron_SOURCE_DIR} EXCLUDE_FROM_ALL)
include_directories(${aeron_SOURCE_DIR}/aeron-client/src/main/c)

set(SOURCE
    src/aeron/aeronmodule.c
    src/aeron/__init__.pyi
    pyproject.toml
    setup.py
    setup.cfg
    MANIFEST.in
    README.md)

add_library(_ EXCLUDE_FROM_ALL ${SOURCE})

add_custom_target(aeron-python ALL)
add_dependencies(aeron-python aeron_static)

install(TARGETS aeron_static LIBRARY DESTINATION lib)
install(DIRECTORY ${aeron_SOURCE_DIR}/aeron-client/src/main/c DESTINATION include/aeron FILES_MATCHING PATTERN "*.h")
