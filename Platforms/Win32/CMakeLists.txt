cmake_minimum_required(VERSION 3.10)

project(Hikali-Win32Platform CXX)

set(INTERFACE 
    interface/Win32Debug.hpp)

set(SOURCE 
    src/Win32Debug.cpp)

add_library(Hikali-Win32Platform ${SOURCE} ${INTERFACE} ${PLATFORM_INTERFACE_HEADERS})
set_common_target_properties(Hikali-Win32Platform)

target_include_directories(Hikali-Win32Platform PUBLIC interface)
target_link_libraries(Hikali-Win32Platform
PUBLIC 
    Hikali-BasicPlatform
    Hikali-PlatformInterface
PRIVATE 
    BuildSettings
    Shlwapi.lib
)

source_group("src" FILES ${SOURCE})
source_group("interface\\win32" FILES ${INTERFACE})
source_group("interface\\common" FILES ${PLATFORM_INTERFACE_HEADERS})

set_target_properties(Hikali-Win32Platform PROPERTIES FOLDER Renderer/Platforms)

install_core_lib(Hikali-Win32Platform)