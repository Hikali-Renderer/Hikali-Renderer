cmake_minimum_required(VERSION 3.10)

project(Main CXX)

if(PLATFORM_WIN32)
    if(HIKALI_BUILD_WIN32_GUI_AS_CONSOLE)
        set(SOURCE main.cpp)
    else()
        set(SOURCE WinMain.cpp)
    endif()

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
    # user32：窗口与 MessageBoxW（GUI 子系统下无控制台时报错用）
    # shell32：CommandLineToArgvW（解析 --config 参数）
    # gdi32：窗口内容暂用 GDI 绘制（后端接入后可去掉）
    target_link_libraries(Main PRIVATE user32 shell32 gdi32)
else()
    message(FATAL_ERROR "Unsupported platform!")
endif()