cmake_minimum_required(VERSION 3.16)
project(MemoryManagement C)

add_library(MemoryManagement
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main_memory.c
)

target_include_directories(MemoryManagement
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(MemoryManagement
    PUBLIC
    log
)

