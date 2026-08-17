cmake_minimum_required(VERSION 3.10)

add_library(Hikali-PlatformInterface INTERFACE)

target_include_directories(Hikali-PlatformInterface INTERFACE interface)

# 一定要写成../interface，直接写interface在Win32目录下的CMakeLists.txt中会报错
# 找不到interface/Win32Debug.hpp
set(PLATFORM_INTERFACE_HEADERS
    ../interface/PlatformDebug.hpp)

add_subdirectory(Basic)

if(PLATFORM_WIN32)
    add_subdirectory(Win32)
    add_library(Hikali-TargetPlatform ALIAS Hikali-Win32Platform)
else()
    message(FATAL_ERROR "Unsupported platform!")
endif()