# Examples CMakeLists.txt

# Basic RTU example
add_executable(basic_rtu basic_rtu/main.c)
target_link_libraries(basic_rtu PRIVATE smartmodbus)

# Basic TCP example
add_executable(basic_tcp basic_tcp/main.c)
target_link_libraries(basic_tcp PRIVATE smartmodbus)

# Advanced merge demonstration
add_executable(advanced_merge advanced_merge/main.c)
target_link_libraries(advanced_merge PRIVATE smartmodbus)

# Embedded static memory example
if(MB_USE_STATIC_MEMORY)
    add_executable(embedded_static embedded_static/main.c)
    target_link_libraries(embedded_static PRIVATE smartmodbus)
endif()

# Installation
install(TARGETS basic_rtu basic_tcp advanced_merge
    RUNTIME DESTINATION bin/examples
)
