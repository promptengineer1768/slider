# cmake/Sanitizers.cmake
# AddressSanitizer (ASan), UndefinedBehaviorSanitizer (UBSan), ThreadSanitizer (TSan)

function(EnableSanitizers target)
    if(NOT MARKDOWN_VIEWER_ENABLE_SANITIZERS)
        return()
    endif()

    if(MSVC)
        target_compile_options(${target} PRIVATE
            /fsanitize=address
        )
        message(STATUS "Sanitizers enabled for ${target}: ASan (MSVC)")
        return()
    endif()

    set(SANITIZER_FLAGS "")

    if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        list(APPEND SANITIZER_FLAGS
            -fsanitize=address
            -fsanitize=undefined
            -fno-omit-frame-pointer
            -fno-common
        )
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            list(APPEND SANITIZER_FLAGS -fno-optimize-sibling-calls)
        endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        list(APPEND SANITIZER_FLAGS
            -fsanitize=address
            -fsanitize=undefined
            -fno-omit-frame-pointer
        )
    endif()

    if(SANITIZER_FLAGS)
        target_compile_options(${target} PRIVATE ${SANITIZER_FLAGS})
        target_link_options(${target} PRIVATE ${SANITIZER_FLAGS})
        message(STATUS "Sanitizers enabled for ${target}: ${SANITIZER_FLAGS}")
    endif()
endfunction()
