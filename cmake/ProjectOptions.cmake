# cmake/ProjectOptions.cmake
# Build options and default settings.

option(MARKDOWN_VIEWER_BUILD_TESTS "Build unit tests" ON)
option(MARKDOWN_VIEWER_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)
option(MARKDOWN_VIEWER_ENABLE_SANITIZERS "Enable sanitizers (ASan, UBSan) in debug builds" OFF)
option(MARKDOWN_VIEWER_ENABLE_I18N "Enable internationalization support" ON)

set(CMAKE_CXX_STANDARD 20 CACHE STRING "C++ standard version")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "Require C++ standard support")
set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "Disable compiler extensions")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json")

if(MSVC AND DEFINED VCPKG_TARGET_TRIPLET AND VCPKG_TARGET_TRIPLET MATCHES "-static$")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING
      "MSVC runtime library" FORCE)
endif()
