cmake_minimum_required(VERSION 3.10)

project(Main CXX)

if(PLATFORM_WIN32)
    if(HIKALI_BUILD_WIN32_GUI_AS_CONSOLE)
        set(SOURCE src/main.cpp)
    else()
        set(SOURCE src/WinMain.cpp)
    endif()
    set(INCLUDE include/Win32/Win32AppBase.hpp)
    list(APPEND SOURCE src/Win32/Win32App.cpp)
else()
    message(FATAL_ERROR "Unsupported platform!")
endif()

list(APPEND INCLUDE 
    include/AppBase.hpp
    include/NativeAppBase.hpp
    include/MainApp.hpp)
list(APPEND SOURCE src/MainApp.cpp)

add_executable(Main ${SOURCE} ${INCLUDE})
set_common_target_properties(Main)

if(MSVC)
    target_compile_options(Main PRIVATE -DUNICODE)
endif()

if(PLATFORM_WIN32)
    if(NOT HIKALI_BUILD_WIN32_GUI_AS_CONSOLE)
    set_target_properties(Main PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()

    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_link_libraries(Main PRIVATE mingw32)
    endif()

    # user32：窗口与 MessageBoxW（GUI 子系统下无控制台时报错用）
    # shell32：CommandLineToArgvW（解析 --config 参数）
    # gdi32：窗口内容暂用 GDI 绘制（后端接入后可去掉）
    target_link_libraries(Main PRIVATE user32 shell32 gdi32)
endif()

target_include_directories(Main PRIVATE include ../../Config)
if(PLATFORM_WIN32)
    target_include_directories(Main PRIVATE include/Win32)
endif()
target_link_libraries(Main PRIVATE RendererConfig)
target_link_libraries(Main PRIVATE BuildSettings)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(Main PROPERTIES FOLDER Renderer)