# Static analysis tools configuration for SmartModbus

option(MB_ENABLE_CPPCHECK "Enable cppcheck static analysis" OFF)
option(MB_ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)

if(MB_ENABLE_CPPCHECK)
    find_program(CPPCHECK cppcheck)
    if(CPPCHECK)
        set(CMAKE_C_CPPCHECK
            ${CPPCHECK}
            --enable=warning,style,performance,portability
            --inline-suppr
            --suppress=missingIncludeSystem
        )
        message(STATUS "cppcheck found: ${CPPCHECK}")
    else()
        message(WARNING "cppcheck requested but not found")
    endif()
endif()

if(MB_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY clang-tidy)
    if(CLANG_TIDY)
        set(CMAKE_C_CLANG_TIDY ${CLANG_TIDY})
        message(STATUS "clang-tidy found: ${CLANG_TIDY}")
    else()
        message(WARNING "clang-tidy requested but not found")
    endif()
endif()
