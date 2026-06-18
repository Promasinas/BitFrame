cmake_minimum_required(VERSION 3.16)
project(BitFream LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Global include directory
include_directories(${CMAKE_SOURCE_DIR}/include)

# ===== MemoryManagement =====
add_library(MemoryManagement
    src/MemoryManagement/src/main_memory.c
)

# Force .c file as C++ since MemoryManagement.h includes <cstdint>
set_source_files_properties(
    src/MemoryManagement/src/main_memory.c
    PROPERTIES LANGUAGE CXX
)

target_include_directories(MemoryManagement
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/MemoryManagement
        ${CMAKE_SOURCE_DIR}/src/MemoryManagement/inc
)

# ===== log.c (rxi/log.c) =====
add_library(log STATIC
    lib/log.c/src/log.c
)
target_include_directories(log PUBLIC
    lib/log.c/src
)

# ===== Tests =====
enable_testing()
add_subdirectory(test/log)
