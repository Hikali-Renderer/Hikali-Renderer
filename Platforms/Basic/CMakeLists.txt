cmake_minimum_required(VERSION 3.10)

project(Hikali-BasicPlatform CXX)

set(SOURCE 
    src/BasicPlatformDebug.cpp)

set(INTERFACE 
    interface/BasicPlatformDebug.hpp
    interface/DebugUtilities.hpp)

add_library(Hikali-BasicPlatform STATIC ${SOURCE} ${INTERFACE})
set_common_target_properties(Hikali-BasicPlatform)

target_include_directories(Hikali-BasicPlatform 
PUBLIC 
    interface
    ../../Primitives/interface)
target_link_libraries(Hikali-BasicPlatform 
PRIVATE 
    BuildSettings
PUBLIC 
    Hikali-Primitives)

source_group("src" FILES ${SOURCE})
source_group("interface" FILES ${INTERFACE})

set_target_properties(Hikali-BasicPlatform PROPERTIES FOLDER Renderer/Platforms)

install_core_lib(Hikali-BasicPlatform)
get_target_relative_dir(Hikali-BasicPlatform BASIC_PLATFORM_DIR)
cmake_path(GET BASIC_PLATFORM_DIR PARENT_PATH PLATFORMS_DIR)
install(
    DIRECTORY "../interface"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PLATFORMS_DIR}"
)