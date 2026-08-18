cmake_minimum_required(VERSION 3.10)

project(Hikali-Primitives CXX)

set(SOURCE 
    src/DebugOutput.cpp)

set(INTERFACE 
    interface/DebugOutput.h
    interface/FormatString.hpp
    interface/Error.hpp
    interface/MemoryAllocator.h
    interface/AlignedMalloc.h
    interface/InterfaceID.h
    interface/ReferenceCounters.h
    interface/Object.h)

add_library(Hikali-Primitives STATIC ${INTERFACE} ${SOURCE})
set_common_target_properties(Hikali-Primitives)

target_link_libraries(Hikali-Primitives PRIVATE BuildSettings)
target_include_directories(Hikali-Primitives PUBLIC interface)

source_group("interface" FILES ${INTERFACE})
source_group("src" FILES ${SOURCE})

set_target_properties(Hikali-Primitives PROPERTIES FOLDER Renderer)

install_core_lib(Hikali-Primitives)