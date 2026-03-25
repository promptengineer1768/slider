# cmake/CompilerWarnings.cmake
# Configures compiler warnings for a target.

function(set_project_warnings target_name)
  option(MARKDOWN_VIEWER_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)

  set(MSVC_WARNINGS
    /W4
    /permissive-
    /utf-8
  )

  set(CLANG_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
    -Wno-unused-function
  )

  set(GCC_WARNINGS
    ${CLANG_WARNINGS}
    -Wno-missing-field-initializers
  )

  if(MARKDOWN_VIEWER_WARNINGS_AS_ERRORS)
    list(APPEND MSVC_WARNINGS /WX)
    list(APPEND CLANG_WARNINGS -Werror)
    list(APPEND GCC_WARNINGS -Werror)
  endif()

  if(MSVC)
    set(PROJECT_WARNINGS ${MSVC_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(PROJECT_WARNINGS ${CLANG_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(PROJECT_WARNINGS ${GCC_WARNINGS})
  else()
    message(WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}'")
  endif()

  target_compile_options(${target_name} PRIVATE ${PROJECT_WARNINGS})
endfunction()
