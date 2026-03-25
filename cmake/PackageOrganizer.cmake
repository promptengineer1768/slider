# cmake/PackageOrganizer.cmake
# Organizes CPack output into dist/platform/ structure

set(DIST_ROOT "${CMAKE_SOURCE_DIR}/dist")

function(organize_packages platform_name)
  set(PLATFORM_DIR "${DIST_ROOT}/${platform_name}")
  file(MAKE_DIRECTORY "${PLATFORM_DIR}")
  
  # Find all generated packages
  file(GLOB packages "${CPACK_PACKAGE_DIRECTORY}/*")
  foreach(pkg ${packages})
    if(IS_DIRECTORY "${pkg}")
      continue()
    endif()
    get_filename_component(filename "${pkg}" NAME)
    message(STATUS "Moving ${filename} to ${PLATFORM_DIR}/")
    file(RENAME "${pkg}" "${PLATFORM_DIR}/${filename}")
  endforeach()
endfunction()

# Call after CPack completes
organize_packages("${CPACK_SYSTEM_NAME}")
