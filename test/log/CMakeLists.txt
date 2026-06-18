# test/log/CMakeLists.txt — log.c unit test
#   log library target is defined in the root CMakeLists.txt

# test executable — output to local build/ within this test folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

add_executable(test_log test_log.c)
target_link_libraries(test_log PRIVATE log)

# register with CTest
add_test(NAME log_test COMMAND test_log)
