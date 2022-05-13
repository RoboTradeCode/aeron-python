cmake_minimum_required(VERSION 3.16)
project(aeron-python C)

include_directories(/usr/include/python3.10)
include_directories(/usr/local/include)

set(SOURCE
        src/aeron/aeronmodule.c
        src/aeron/__init__.pyi
        pyproject.toml
        setup.py
        setup.cfg
        MANIFEST.in
        README.md)

add_library(_ EXCLUDE_FROM_ALL ${SOURCE})
