# Unity test framework
add_library(unity unity/unity.c)
target_include_directories(unity PUBLIC unity)

# Helper function to add tests
function(add_smartmodbus_test test_name)
    add_executable(${test_name} unit/${test_name}.c)
    target_link_libraries(${test_name} PRIVATE smartmodbus unity)
    target_include_directories(${test_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Unit tests
add_smartmodbus_test(test_crc16)
add_smartmodbus_test(test_lrc)
add_smartmodbus_test(test_rtu_frame)
add_smartmodbus_test(test_ascii_frame)
add_smartmodbus_test(test_tcp_frame)
add_smartmodbus_test(test_cost_model)
add_smartmodbus_test(test_gap_merge)
add_smartmodbus_test(test_ffd_pack)
add_smartmodbus_test(test_block_utils)
add_smartmodbus_test(test_response_parser)

message(STATUS "Unit tests configured with Unity framework")
