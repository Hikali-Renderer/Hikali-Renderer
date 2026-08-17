cmake_minimum_required(VERSION 3.10)

project(Hikali-Common CXX)

set(INCLUDE include/pch.h)

set(INTERFACE 
    interface/RefCntAutoPtr.hpp)

set(SOURCE 
    src/SpinLock.cpp)

add_library(Hikali-Common STATIC ${SOURCE} ${INCLUDE} ${INTERFACE})

target_include_directories(Hikali-Common PRIVATE include PUBLIC interface)
target_link_libraries(Hikali-Common PRIVATE BuildSettings)

set_common_target_properties(Hikali-Common)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})
source_group("interface" FILES ${INTERFACE})

set_target_properties(Hikali-Common PROPERTIES FOLDER Renderer)

install_core_lib(Hikali-Common)