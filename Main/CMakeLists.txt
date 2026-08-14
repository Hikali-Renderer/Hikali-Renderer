cmake_minimum_required(VERSION 3.10)

project(Main CXX)

if(PLATFORM_WIN32)
    set(SOURCE main.cpp)

    add_executable(Main ${SOURCE})
    if(NOT HIKALI_BUILD_WIN32_GUI_AS_CONSOLE)
        set_target_properties(Main PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
    set_target_properties(Main PROPERTIES FOLDER Renderer)

    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_link_libraries(Main PRIVATE mingw32)
    endif()

    target_include_directories(Main PRIVATE ../../Config)
    target_link_libraries(Main PRIVATE RendererConfig)
    target_link_libraries(Main PRIVATE BuildSettings)
else()
    message(FATAL_ERROR "Unsupported platform!")
endif()