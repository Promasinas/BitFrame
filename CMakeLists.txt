cmake_minimum_required(VERSION 3.16)
project(BitFream
    VERSION 1.0.0
    DESCRIPTION "1-bit boolean convolution framework"
    LANGUAGES C CXX
)


add_executable(BitFream
    src/main.c
)

add_subdirectory(src/MemoryManagement)
add_subdirectory(lib/log)