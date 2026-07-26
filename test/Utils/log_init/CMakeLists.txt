# test/Utils/log_init/CMakeLists.txt
#   Utils library target is defined in the root CMakeLists.txt

# test executable — output to local build/ within this test folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

add_executable(test_log_init test_log_init.c)
target_link_libraries(test_log_init PRIVATE Utils)

# register with CTest
add_test(NAME LogInit COMMAND test_log_init)
