# cmake/LinkTimeOptimization.cmake
# Link-Time Optimization (LTO) / Interprocedural Optimization (IPO)

function(EnableLTO target)
  if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    return()
  endif()

  include(CheckIPOSupported)
  check_ipo_supported(RESULT lto_supported OUTPUT lto_output)

  if(lto_supported)
    set_target_properties(${target} PROPERTIES
      INTERPROCEDURAL_OPTIMIZATION ON
    )
    message(STATUS "LTO/IPO enabled for ${target}")
  else()
    message(WARNING "LTO not supported: ${lto_output}")
  endif()
endfunction()
