# test/MemoryManagement/main_memory/CMakeLists.txt
#   MemoryManagement library target is defined in the root CMakeLists.txt

# test executable — output to local build/ within this test folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

add_executable(test_main_memory test_main_memory.c)
target_link_libraries(test_main_memory PRIVATE MemoryManagement)

# register with CTest
add_test(NAME MainMemory COMMAND test_main_memory)
