# cmake/Dependencies.cmake
# Find and configure external dependencies.

if(WIN32)
    find_package(wxWidgets CONFIG REQUIRED COMPONENTS base core adv)
else()
    find_package(wxWidgets QUIET COMPONENTS base core adv)

    if(wxWidgets_FOUND)
        if(DEFINED wxWidgets_USE_FILE AND EXISTS "${wxWidgets_USE_FILE}")
            include(${wxWidgets_USE_FILE})
        endif()
    else()
        find_package(wxWidgets REQUIRED COMPONENTS std adv)
    endif()
endif()

if(SLIDER_BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED)
endif()

