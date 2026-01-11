cmake_minimum_required(VERSION 3.15)
project(SmartModbus VERSION 1.0.0 LANGUAGES C)

# C11 standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build options
option(MB_BUILD_TESTS "Build unit tests" ON)
option(MB_BUILD_EXAMPLES "Build example applications" ON)
option(MB_USE_STATIC_MEMORY "Use static memory allocation" OFF)
option(MB_ENABLE_RTU "Enable RTU support" ON)
option(MB_ENABLE_ASCII "Enable ASCII support" ON)
option(MB_ENABLE_TCP "Enable TCP/IP support" ON)

# Configuration parameters
set(MB_MAX_PDU_CHARS 253 CACHE STRING "Maximum PDU size in characters")
set(MB_MAX_BLOCKS 32 CACHE STRING "Maximum blocks (static memory mode)")
set(MB_MAX_PDUS 16 CACHE STRING "Maximum PDUs (static memory mode)")
set(MB_MAX_PLANS 16 CACHE STRING "Maximum request plans (static memory mode)")

# Include CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Library target
add_subdirectory(src)

# Tests
if(MB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(MB_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation
install(DIRECTORY include/ DESTINATION include)

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/SmartModbusConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/SmartModbusConfigVersion.cmake"
    DESTINATION lib/cmake/SmartModbus
)
