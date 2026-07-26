cmake_minimum_required(VERSION 3.16)
project(BitFream
    VERSION 1.0.0
    DESCRIPTION "1-bit boolean convolution framework"
    LANGUAGES C CXX
)

# ================================================================
# Options
# ================================================================
option(BUILD_TESTS    "Build test executables"     ON)
option(BUILD_MAIN     "Build main executable"      ON)
option(USE_POPCNT     "Use hardware POPCNT (x86)"  ON)
option(USE_AVX2       "Use AVX2 instructions"      OFF)

# ================================================================
# System detection
# ================================================================
if(WIN32)
    set(BF_OS_WINDOWS TRUE)
    set(BF_OS_NAME   "Windows")
elseif(APPLE)
    set(BF_OS_MACOS   TRUE)
    set(BF_OS_NAME    "macOS")
elseif(UNIX)
    set(BF_OS_LINUX   TRUE)
    set(BF_OS_NAME    "Linux")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BF_ARCH "x86_64")
else()
    set(BF_ARCH "x86")
endif()

message(STATUS "BitFream — target: ${BF_OS_NAME} / ${BF_ARCH}")

# ================================================================
# Output directories
# ================================================================
# ================================================================
# C/C++ standards
# ================================================================
set(CMAKE_C_STANDARD          11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD        17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ================================================================
# Compiler flags per toolchain
# ================================================================
if(MSVC)
    # ----- MSVC -----
    if(USE_POPCNT)
        add_compile_options(/arch:AVX2)
    endif()
else()
    # ----- GCC / Clang -----
    add_compile_options(-Wall -Wextra -fvisibility=hidden)
    if(USE_POPCNT)
        if(BF_ARCH STREQUAL "x86_64")
            add_compile_options(-mpopcnt -msse4.2)
        endif()
    endif()
    if(USE_AVX2 AND BF_ARCH STREQUAL "x86_64")
        add_compile_options(-mavx2)
    endif()
endif()

# Debug / Release
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MSVC)
        add_compile_options(/Od /Zi)
    else()
        add_compile_options(-Og -g)
        add_compile_definitions(LOG_USE_COLOR)
    endif()
else()
    if(MSVC)
        add_compile_options(/O2 /DNDEBUG)
    else()
        add_compile_options(-DNDEBUG)
    endif()
endif()

# Platform defines
if(BF_OS_WINDOWS)
    add_compile_definitions(BF_PLATFORM_WINDOWS)
elseif(BF_OS_MACOS)
    add_compile_definitions(BF_PLATFORM_MACOS)
elseif(BF_OS_LINUX)
    add_compile_definitions(BF_PLATFORM_LINUX)
endif()
if(BF_ARCH STREQUAL "x86_64")
    add_compile_definitions(BF_ARCH_X86_64)
endif()

# ================================================================
# Global include
# ================================================================
include_directories(${CMAKE_SOURCE_DIR}/include)

# ================================================================
# Third-party: log.c (rxi/log.c — MIT)  — 静态库
# ================================================================
add_library(log STATIC
    lib/log.c/src/log.c
)
target_include_directories(log PUBLIC
    lib/log.c/src
)

# ================================================================
# Shared library helper — 跨平台导出宏
# ================================================================
# 生成头文件到 build 目录，由各模块 include
if(BF_OS_WINDOWS)
    set(BF_EXPORT_DEFINE "__declspec(dllexport)")
    set(BF_IMPORT_DEFINE "__declspec(dllimport)")
else()
    set(BF_EXPORT_DEFINE "__attribute__((visibility(\"default\")))")
    set(BF_IMPORT_DEFINE "__attribute__((visibility(\"default\")))")
endif()

# ================================================================
# Library: MemoryManagement  (shared → .dll / .so / .dylib)
# ================================================================
add_library(MemoryManagement SHARED
    src/MemoryManagement/src/main_memory.c
)
target_include_directories(MemoryManagement
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/MemoryManagement
        ${CMAKE_SOURCE_DIR}/src/MemoryManagement/inc
)
target_link_libraries(MemoryManagement PUBLIC log)
target_compile_definitions(MemoryManagement PRIVATE BF_MEMORYMANAGEMENT_EXPORTS)

# ================================================================
# Library: Utils  (shared → .dll / .so / .dylib)
# ================================================================
add_library(Utils SHARED
    src/Utils/src/log_init.c
)
target_include_directories(Utils
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/Utils
        ${CMAKE_SOURCE_DIR}/src/Utils/inc
)
target_link_libraries(Utils PUBLIC log)
target_compile_definitions(Utils PRIVATE BF_UTILS_EXPORTS)

# ================================================================
# Library: OperatorKernals  (shared → .dll / .so / .dylib)
# ================================================================
add_library(OperatorKernals SHARED
    src/OperatorKernals/src/bool_conv_1d.c
    src/OperatorKernals/src/bool_conv_1d_upsample.c
)
target_include_directories(OperatorKernals
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/OperatorKernals
        ${CMAKE_SOURCE_DIR}/src/OperatorKernals/inc
)
target_link_libraries(OperatorKernals PUBLIC log)
target_compile_definitions(OperatorKernals PRIVATE BF_OPERATORKERNALS_EXPORTS)

# ================================================================
# Library: RuntimeManagement  (shared → .dll / .so / .dylib)
# ================================================================
add_library(RuntimeManagement SHARED
    src/RuntimeManagement/src/network_forward.c
    src/RuntimeManagement/src/network_testlin_train.c
)
target_include_directories(RuntimeManagement
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/RuntimeManagement
        ${CMAKE_SOURCE_DIR}/src/RuntimeManagement/inc
)
target_compile_definitions(RuntimeManagement PRIVATE BF_RUNTIME_EXPORTS)

# ================================================================
# Main executable
# ================================================================
if(BUILD_MAIN)
    add_executable(BitFream src/main.c)
    target_link_libraries(BitFream PRIVATE
        MemoryManagement
        OperatorKernals
        Utils
    )
endif()

# ================================================================
# Tests
# ================================================================
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test/lib/log)
    add_subdirectory(test/OperatorKernals/bool_conv_1d)
    add_subdirectory(test/OperatorKernals/bool_conv_1d_upsample)
    add_subdirectory(test/MemoryManagement/main_memory)
    add_subdirectory(test/Utils/log_init)
    add_subdirectory(test/RuntimeManagement/network_forward)
    add_subdirectory(test/RuntimeManagement/network_testlin_train)
endif()

# ================================================================
# Summary
# ================================================================
message(STATUS "")
message(STATUS "BitFream ${PROJECT_VERSION}")
message(STATUS "  OS:        ${BF_OS_NAME}")
message(STATUS "  Arch:      ${BF_ARCH}")
message(STATUS "  Build:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  Tests:     ${BUILD_TESTS}")
message(STATUS "  Main:      ${BUILD_MAIN}")
message(STATUS "  POPCNT:    ${USE_POPCNT}")
message(STATUS "  AVX2:      ${USE_AVX2}")
message(STATUS "  Output:    build/")
message(STATUS "")
