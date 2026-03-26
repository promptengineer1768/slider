cmake_minimum_required(VERSION 3.28)
project(slider VERSION 1.0.1 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(ProjectOptions)
include(Dependencies)
include(CompilerWarnings)
include(Versioning)
include(Sanitizers)
include(CodeCoverage)
include(LinkTimeOptimization)

set(SLIDER_CORE_SOURCES
    src/core/board.cc
    src/core/solver.cc
    src/core/scrambler.cc
    src/core/state.cc
    src/core/savefile.cc
)

add_library(slider_core STATIC ${SLIDER_CORE_SOURCES})
target_include_directories(slider_core PUBLIC include)
target_compile_features(slider_core PUBLIC cxx_std_20)
set_project_warnings(slider_core)
EnableSanitizers(slider_core)
EnableCoverage(slider_core)
EnableLTO(slider_core)
GenerateVersionHeader(slider_core)

if(WIN32)
    target_compile_definitions(slider_core PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()
if(MSVC)
    target_compile_options(slider_core PRIVATE /utf-8)
endif()

file(COPY
    "${CMAKE_SOURCE_DIR}/resources/slide.wav"
    "${CMAKE_SOURCE_DIR}/resources/complete.wav"
    "${CMAKE_SOURCE_DIR}/resources/themes.json"
    DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources"
)

if(WIN32)
    file(COPY "${CMAKE_SOURCE_DIR}/resources/icon.ico" DESTINATION "${CMAKE_BINARY_DIR}")
    set(ICON_PATH "icon.ico")
    configure_file(src/app/slider.rc.in "${CMAKE_BINARY_DIR}/slider.rc")
    add_executable(slider WIN32
        src/app/wx_app.cc
        src/app/game_controller.cc
        "${CMAKE_BINARY_DIR}/slider.rc"
    )
else()
    add_executable(slider
        src/app/wx_app.cc
        src/app/game_controller.cc
    )
    if(APPLE)
        set_target_properties(slider PROPERTIES MACOSX_BUNDLE TRUE)
    endif()
endif()

if(TARGET wx::core AND TARGET wx::base AND TARGET wx::adv)
    target_link_libraries(slider PRIVATE slider_core wx::core wx::base wx::adv)
else()
    target_link_libraries(slider PRIVATE slider_core ${wxWidgets_LIBRARIES})
endif()
set_project_warnings(slider)
EnableSanitizers(slider)
EnableCoverage(slider)
EnableLTO(slider)
GenerateVersionHeader(slider)

if(MSVC)
    target_compile_options(slider PRIVATE /utf-8)
endif()

if(SLIDER_BUILD_TESTS)
    enable_testing()
    include(GoogleTest)

    add_executable(slider_tests
        tests/unit/board_test.cc
        tests/unit/game_controller_test.cc
        tests/unit/solver_test.cc
        tests/unit/scrambler_test.cc
        tests/unit/gui_logic_test.cc
        tests/unit/serialization_test.cc
        tests/unit/state_validity_test.cc
        tests/unit/savefile_test.cc
        src/app/game_controller.cc
    )
    target_link_libraries(slider_tests
        PRIVATE GTest::gtest_main GTest::gtest slider_core
    )
    set_project_warnings(slider_tests)
    EnableSanitizers(slider_tests)
    EnableCoverage(slider_tests)
    if(MSVC)
        target_compile_options(slider_tests PRIVATE /utf-8)
    endif()

    gtest_discover_tests(slider_tests)
    AddCoverageTarget()
endif()

if(TARGET slider)
    if(WIN32)
        install(TARGETS slider RUNTIME DESTINATION bin)
    else()
        install(TARGETS slider RUNTIME DESTINATION bin BUNDLE DESTINATION .)
    endif()

    if(WIN32)
        # Only ship runtime assets in bin/resources on Windows (avoid duplicating icon sources).
        install(FILES
            ${CMAKE_SOURCE_DIR}/resources/slide.wav
            ${CMAKE_SOURCE_DIR}/resources/complete.wav
            ${CMAKE_SOURCE_DIR}/resources/themes.json
            DESTINATION bin/resources
        )
    else()
        # Keep full resources directory on non-Windows for now.
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/ DESTINATION bin/resources)
    endif()

    if(WIN32)
        install(FILES ${CMAKE_SOURCE_DIR}/resources/icon.ico DESTINATION bin)
    endif()
endif()

include(Packaging)
