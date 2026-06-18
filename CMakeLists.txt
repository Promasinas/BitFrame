cmake_minimum_required(VERSION 3.16)
project(BitFream LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Global include directory
include_directories(${CMAKE_SOURCE_DIR}/include)

# ===== log.c (rxi/log.c) =====
add_library(log STATIC
    lib/log.c/src/log.c
)
target_include_directories(log PUBLIC
    lib/log.c/src
)

# ===== MemoryManagement =====
add_library(MemoryManagement
    src/MemoryManagement/src/main_memory.c
)

target_include_directories(MemoryManagement
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/MemoryManagement
        ${CMAKE_SOURCE_DIR}/src/MemoryManagement/inc
)

target_link_libraries(MemoryManagement PUBLIC log)

# ===== Utils =====
add_library(Utils
    src/Utils/src/log_init.c
)

target_include_directories(Utils
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/Utils
        ${CMAKE_SOURCE_DIR}/src/Utils/inc
)

target_link_libraries(Utils PUBLIC log)

# ===== Main executable =====
add_executable(BitFream src/main.c)
set_target_properties(BitFream PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)
target_link_libraries(BitFream PRIVATE MemoryManagement Utils log)

# ===== Tests =====
enable_testing()
add_subdirectory(test/log)
