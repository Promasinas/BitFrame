cmake_minimum_required(VERSION 3.16)
project(log C)

add_library(log
    ${CMAKE_CURRENT_SOURCE_DIR}/src/log.c
)

target_include_directories(log
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

