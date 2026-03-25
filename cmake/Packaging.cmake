# cmake/Packaging.cmake
# CPack configuration for binary packaging.

set(CPACK_PACKAGE_NAME "slider")
set(CPACK_PACKAGE_VENDOR "Sliding Puzzle Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A cross-platform sliding puzzle game.")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Slider")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Output packages to dist/ directory with platform subdirectories
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_SOURCE_DIR}/dist")

if(WIN32)
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
  set(CPACK_GENERATOR "ZIP")
  
  find_program(WIX_CANDLE candle.exe PATHS 
    "C:/Program Files (x86)/WiX Toolset v3.14/bin"
    "C:/Program Files/WiX Toolset v3.14/bin"
    "C:/Program Files/WiX Toolset v4.0/bin"
    ENV PATH
  )
  if(WIX_CANDLE)
    message(STATUS "WiX Toolset found, enabling MSI packaging")
    list(APPEND CPACK_GENERATOR "WIX")
  else()
    message(STATUS "WiX Toolset not found, MSI packaging disabled")
  endif()
  
  set(CPACK_WINDOWS_PACKAGE_NAME "Sliding Puzzle")

  set(CPACK_WIX_UPGRADE_GUID "e8d2e8b6-94e2-45a7-93e1-32c589b9f911")
  set(CPACK_WIX_PRODUCT_NAME "Sliding Puzzle")
  set(CPACK_WIX_PROGRAM_ICON "${CMAKE_SOURCE_DIR}/resources/icon.ico")
  set(CPACK_WIX_UI "WixUI_InstallDir")
  set(CPACK_WIX_PROPERTY_ALLUSERS "1")
elseif(APPLE)
  set(CPACK_GENERATOR "DragNDrop;TGZ")
  if(TARGET slider)
    set_target_properties(slider PROPERTIES
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_BUNDLE_NAME "Sliding Puzzle"
      MACOSX_BUNDLE_GUI_IDENTIFIER "com.slider.game"
      MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
      MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2026 Sliding Puzzle Team"
    )
  endif()
else()
  set(CPACK_GENERATOR "DEB;RPM;TGZ")
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Sliding Puzzle Team <halifaxgeorge@gmail.com>")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libwxgtk3.2-1")
  set(CPACK_RPM_PACKAGE_LICENSE "MIT")
  set(CPACK_RPM_PACKAGE_REQUIRES "wxGTK3")
  set(CPACK_RPM_PACKAGE_GROUP "Games")
endif()

include(CPack)
