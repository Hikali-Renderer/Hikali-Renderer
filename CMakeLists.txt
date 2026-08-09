cmake_minimum_required (VERSION 3.10)

add_executable(Hikali WIN32 main.cpp)

target_link_libraries(Hikali PRIVATE RendererConfig)