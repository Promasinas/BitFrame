# test/OperatorKernals/bool_conv_1d/CMakeLists.txt
#   OperatorKernals library target is defined in the root CMakeLists.txt

# test executable — output to local build/ within this test folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

add_executable(test_bool_conv_1d test_bool_conv_1d.c)
target_link_libraries(test_bool_conv_1d PRIVATE OperatorKernals)

# register with CTest
add_test(NAME BoolConv1D COMMAND test_bool_conv_1d)
