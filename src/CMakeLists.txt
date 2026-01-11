# Collect source files
set(SMARTMODBUS_SOURCES
    core/char_model.c
    core/gap_merge.c
    core/ffd_pack.c
    core/fc_policy.c
    master/master_api.c
    master/request_optimizer.c
    master/response_parser.c
    utils/block_utils.c
)

# Protocol sources - conditional compilation
if(MB_ENABLE_RTU)
    list(APPEND SMARTMODBUS_SOURCES
        protocol/rtu_frame.c
        protocol/crc16.c
    )
endif()

if(MB_ENABLE_ASCII)
    list(APPEND SMARTMODBUS_SOURCES
        protocol/ascii_frame.c
        protocol/lrc.c
    )
endif()

if(MB_ENABLE_TCP)
    list(APPEND SMARTMODBUS_SOURCES
        protocol/tcp_frame.c
    )
endif()

# Frame builder (always included)
list(APPEND SMARTMODBUS_SOURCES protocol/frame_builder.c)

# Memory pool for static memory mode
if(MB_USE_STATIC_MEMORY)
    list(APPEND SMARTMODBUS_SOURCES utils/memory_pool.c)
endif()

# Create library
add_library(smartmodbus ${SMARTMODBUS_SOURCES})

# Include directories
target_include_directories(smartmodbus
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compile definitions
if(MB_USE_STATIC_MEMORY)
    target_compile_definitions(smartmodbus PUBLIC
        MB_USE_STATIC_MEMORY
        MB_MAX_BLOCKS=${MB_MAX_BLOCKS}
        MB_MAX_PDUS=${MB_MAX_PDUS}
        MB_MAX_PLANS=${MB_MAX_PLANS}
    )
endif()

if(MB_ENABLE_RTU)
    target_compile_definitions(smartmodbus PUBLIC MB_ENABLE_RTU)
endif()

if(MB_ENABLE_ASCII)
    target_compile_definitions(smartmodbus PUBLIC MB_ENABLE_ASCII)
endif()

if(MB_ENABLE_TCP)
    target_compile_definitions(smartmodbus PUBLIC MB_ENABLE_TCP)
endif()

target_compile_definitions(smartmodbus PUBLIC MB_MAX_PDU_CHARS=${MB_MAX_PDU_CHARS})

# Compiler warnings
include(CompilerWarnings)
set_project_warnings(smartmodbus)

# Installation
install(TARGETS smartmodbus
    EXPORT SmartModbusTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(EXPORT SmartModbusTargets
    FILE SmartModbusTargets.cmake
    NAMESPACE SmartModbus::
    DESTINATION lib/cmake/SmartModbus
)
