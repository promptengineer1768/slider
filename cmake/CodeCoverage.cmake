# cmake/CodeCoverage.cmake
# Code coverage configuration using gcov/lcov or coverage.py

option(MARKDOWN_VIEWER_ENABLE_COVERAGE "Enable code coverage" OFF)

function(EnableCoverage target)
  if(NOT MARKDOWN_VIEWER_ENABLE_COVERAGE)
    return()
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    target_compile_options(${target} PRIVATE
      --coverage
      -fprofile-arcs
      -ftest-coverage
      -fprofile-abs-path
    )
    target_link_options(${target} PRIVATE --coverage)
    message(STATUS "Code coverage enabled for ${target}")
  elseif(MSVC)
    message(WARNING "Code coverage not supported with MSVC. Use OpenCppCoverage or similar tools.")
  endif()
endfunction()

function(AddCoverageTarget)
  if(NOT MARKDOWN_VIEWER_ENABLE_COVERAGE)
    return()
  endif()

  find_program(LCOV_PATH lcov)
  find_program(GENHTML_PATH genhtml)

  if(LCOV_PATH AND GENHTML_PATH)
    add_custom_target(coverage
      COMMAND ${LCOV_PATH} --capture --directory . --output-file coverage.info
      COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' --output-file coverage.info.cleaned
      COMMAND ${GENHTML_PATH} -o coverage coverage.info.cleaned
      COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated in coverage/index.html"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Generating code coverage report"
      VERBATIM
    )
    message(STATUS "Coverage target added. Run 'cmake --build . --target coverage' to generate report.")
  else()
    message(WARNING "lcov/genhtml not found. Coverage target not available.")
  endif()
endfunction()
