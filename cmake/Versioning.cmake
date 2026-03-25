# cmake/Versioning.cmake
# Generate version header from project version.

function(GenerateVersionHeader target)
    set(VERSION_HEADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
    set(VERSION_HEADER_FILE ${VERSION_HEADER_DIR}/version.h)

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.h.in
        ${VERSION_HEADER_FILE}
        @ONLY
    )

    target_include_directories(${target} PRIVATE ${VERSION_HEADER_DIR})
    target_compile_definitions(${target} PRIVATE
        PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        PROJECT_VERSION="${PROJECT_VERSION}"
    )
endfunction()
