# Compiler warnings configuration for SmartModbus

function(set_project_warnings target)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wcast-align
            -Wunused
            -Wconversion
            -Wsign-conversion
            -Wdouble-promotion
            -Wformat=2
            -Wimplicit-fallthrough
        )

        # Additional warnings for GCC
        if(CMAKE_C_COMPILER_ID MATCHES "GNU")
            target_compile_options(${target} PRIVATE
                -Wlogical-op
                -Wduplicated-cond
                -Wduplicated-branches
            )
        endif()
    elseif(MSVC)
        target_compile_options(${target} PRIVATE
            /W4
            /WX-  # Don't treat warnings as errors by default
        )
    endif()
endfunction()
