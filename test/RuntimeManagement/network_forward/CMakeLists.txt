# test/RuntimeManagement/network_forward/CMakeLists.txt

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

add_executable(test_network_forward test_network_forward.c)
target_link_libraries(test_network_forward PRIVATE RuntimeManagement)

add_test(NAME NetworkForward COMMAND test_network_forward)
