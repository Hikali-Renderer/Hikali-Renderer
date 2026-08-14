cmake_minimum_required (VERSION 3.19)

project(Hikali-Renderer)

# 定义GNU标准安装目录，如CMAKE_INSTALL_INCLUDEDIR, CMAKE_INSTALL_LIBDIR, etc.
include(GNUInstallDirs)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(D3D12_SUPPORTED  FALSE CACHE INTERNAL "D3D12 not supported")
set(VULKAN_SUPPORTED FALSE CACHE INTERNAL "Vulkan not supported")

set(HIKALI_RENDERER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Hikali Renderer module source directory")

set(CMAKE_OBJECT_PATH_MAX 4096)

if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    set(ARCH 64 CACHE INTERNAL "Only support 64-bit architecture")
else()
    message(FATAL_ERROR "Unsupported architecture!")
endif()

if(CMAKE_SYSTEM_PROCESSOR)
    if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64") OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "amd64") OR
        ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "X86_64") OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "AMD64"))
        set(TARGET_CPU x86_64 CACHE INTERNAL "Only support x86_64 CPU")
    else()
        message(FATAL_ERROR "Unsupported CPU!")
    endif()
else()
    message(FATAL_ERROR "Unable to detect target processor: CMAKE_SYSTEM_PROCESSOR is not defined!")
endif()

message("CMake generator: " ${CMAKE_GENERATOR})

if(WIN32)
    include(BuildTools/CMake/FindWindowsSDK.cmake)
    find_windows_sdk()

    if(${CMAKE_SYSTEM_NAME} STREQUAL "WindowsStore")
        message(FATAL_ERROR, "WindowsStore not supported!")
    else()
        set(PLATFORM_WIN32 TRUE CACHE INTERNAL "Target platform: Win32")
        message("Target platform: Win32 " ${ARCH})
    endif()
else()
    message(FATAL_ERROR "Unsupported platform!")
endif(WIN32)

add_library(BuildSettings INTERFACE)

if(PLATFORM_WIN32)
    try_compile(HAS_D3D12 "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/BuildTools/CMake/CheckD3D12.cpp")
    try_compile(HAS_ATL "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/BuildTools/CMake/CheckATL.cpp")
    message("")
    message("Checking for Direct3D12 support:")
    message("  d3d12.h found:   " ${HAS_D3D12})
    message("  atlbase.h found: " ${HAS_ATL})
endif()

if(MINGW)
    message("Building with MinGW")
    if(MSYS)
        message("  in MSYS environment")
    endif()
    set(MINGW_BUILD TRUE CACHE INTERNAL "Building with MinGW")
endif()

if(PLATFORM_WIN32)
    set(VULKAN_SUPPORTED TRUE CACHE INTERNAL "Vulkan is supported on Win32 platform")
    target_compile_definitions(BuildSettings INTERFACE PLATFORM_WIN32=1)
else()
    message(FATAL_ERROR "Unsupported platform!")
endif()

option(HIKALI_NO_HLSL              "Disable HLSL support in non-Direct3D backends" OFF)
option(HIKALI_NO_FORMAT_VALIDATION "Disable source code format validation" OFF)
option(HIKALI_NO_DIRECT3D12        "Disable Direct3D12 backend" OFF)
option(HIKALI_NO_VULKAN            "Disable Vulkan backend" OFF)

if(${HIKALI_NO_DIRECT3D12})
    set(D3D12_SUPPORTED FALSE CACHE INTERNAL "D3D12 backend is forcibly disabled")
endif()
if(${HIKALI_NO_VULKAN})
    set(VULKAN_SUPPORTED FALSE CACHE INTERNAL "Vulkan backend is forcily disabled")
endif()

if(NOT (${D3D12_SUPPORTED} OR ${VULKAN_SUPPORTED}))
    message(FATAL_ERROR "No rendering backends are selected to build")
endif()

message("")
message("D3D12_SUPPORTED:  " ${D3D12_SUPPORTED})
message("VULKAN_SUPPORTED: " ${VULKAN_SUPPORTED})
message("")

target_compile_definitions(BuildSettings INTERFACE 
    D3D12_SUPPORTED=$<BOOL:${D3D12_SUPPORTED}>
    VULKAN_SUPPORTED=$<BOOL:${VULKAN_SUPPORTED}>
)

set(RELEASE_CONFIGURATIONS RELEASE RELWITHDEBINFO MINSIZEREL CACHE INTERNAL "Release configurations")

target_compile_definitions(BuildSettings INTERFACE $<$<CONFIG:DEBUG>:_DEBUG;DEBUG>)
foreach(REL_CONFIG ${RELEASE_CONFIGURATIONS})
    target_compile_definitions(BuildSettings INTERFACE "$<$<CONFIG:${REL_CONFIG}>:NDEBUG>")
endforeach()

if(MSVC)
    # 警告视为错误
    set(HIKALI_MSVC_COMPILE_OPTIONS "" CACHE INTERNAL "Common MSVC compile options")
    if(HIKALI_MSVC_COMPILE_OPTIONS)
        message("Additional MSVC compile options: " ${HIKALI_MSVC_COMPILE_OPTIONS})
    endif()

    set(HIKALI_MSVC_DEBUG_COMPILE_OPTIONS "" CACHE INTERNAL "Additional MSVC compile options for debug configuration")
    if(HIKALI_MSVC_DEBUG_COMPILE_OPTIONS)
        message("Additional MSVC compile options for debug configuration: " ${HIKALI_MSVC_DEBUG_COMPILE_OPTIONS})
    endif()

    # 开启全程序优化
    set(DEFAULT_HIKALI_MSVC_RELEASE_COMPILE_OPTIONS "")
    if("${TARGET_CPU}" STREQUAL "x86_64")
        # 启用AVX2
        list(APPEND DEFAULT_HIKALI_MSVC_RELEASE_COMPILE_OPTIONS /arch:AVX2)
    endif()
    set(HIKALI_MSVC_RELEASE_COMPILE_OPTIONS ${DEFAULT_HIKALI_MSVC_RELEASE_COMPILE_OPTIONS} CACHE STRING "Additional MSVC compile options for release configurations")
    if(HIKALI_MSVC_RELEASE_COMPILE_OPTIONS)
        message("Additional MSVC compile options for release configurations: " ${HIKALI_MSVC_RELEASE_COMPILE_OPTIONS})
    endif()

    # 启用 4 级警告，但禁用
    # - w4100 - 未引用的形参
    # - w4201 - 使用非标准扩展：无名结构体 / 联合体
    # - w4505 - 未引用的本地函数已被移除
    # - C26812 - 优先使用「强枚举 (enum class)」而非普通枚举 (enum)（代码分析警告）
    target_compile_options(BuildSettings INTERFACE /W4 /wd4100 /wd4201 /wd4505 /wd26812 /MP ${HIKALI_MSVC_COMPILE_OPTIONS})

    if(PLATFORM_WIN32)
        # 启用标准兼容模式
        # 注意：通用Windows平台(UWP)源码无法在标准兼容模式下编译
        target_compile_options(BuildSettings INTERFACE /permissive-)
    endif()

    # 在所有发布版本模式下同时启用：
    # - 禁用 w4189 警告 —— 已初始化但未使用的局部变量
    # - 禁用运行时类型信息（/GR-）
    # - 启用字符串池优化（/GF）
    set(MSVC_ALL_RELEASE_COMPILE_OPTIONS /wd4189 /GR- /GF)

    # 发布模式（RELEASE）下：
    # - 启用优先生成快速代码选项（/Ot）
    # - 启用内部函数优化（/Oi）
    # - 最大化运行速度优化（/O2）
    # - 对所有合适的函数启用内联（/Ob2）
    set(MSVC_RELEASE_COMPILE_OPTIONS ${MSVC_ALL_RELEASE_COMPILE_OPTIONS} /Ot /Oi /Ob2 /O2 ${HIKALI_MSVC_RELEASE_COMPILE_OPTIONS})
    set(MSVC_RELWITHDEBINFO_COMPILE_OPTIONS ${MSVC_RELEASE_COMPILE_OPTIONS})
    # MINSIZEREL模式下倾向于 (/Os)
    set(MSVC_MINSIZEREL_COMPILE_OPTIONS ${MSVC_ALL_RELEASE_COMPILE_OPTIONS} /Os)
    target_compile_options(BuildSettings INTERFACE "$<$<CONFIG:DEBUG>:${HIKALI_MSVC_DEBUG_COMPILE_OPTIONS}>")
    target_compile_options(BuildSettings INTERFACE "$<$<CONFIG:RELEASE>:${MSVC_RELEASE_COMPILE_OPTIONS}>")
    target_compile_options(BuildSettings INTERFACE "$<$<CONFIG:MINSIZEREL>:${MSVC_MINSIZEREL_COMPILE_OPTIONS}>")
    target_compile_options(BuildSettings INTERFACE "$<$<CONFIG:RELWITHDEBINFO>:${MSVC_RELWITHDEBINFO_COMPILE_OPTIONS}>")
else()
    target_compile_definitions(BuildSettings INTERFACE __forceinline=inline)
endif(MSVC)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # 警告视为错误
    set(HIKALI_CLANG_COMPILE_OPTIONS "" CACHE STRING "Common Clang compile options")
    if(HIKALI_CLANG_COMPILE_OPTIONS)
        message("Additional Clang compile options: " ${HIKALI_CLANG_COMPILE_OPTIONS})
    endif()

    set(HIKALI_CLANG_DEBUG_COMPILE_OPTIONS "" CACHE STRING "Additional Clang compile options for debug configuration")
    if (HIKALI_CLANG_DEBUG_COMPILE_OPTIONS)
        message("Additional Clang compile options for debug configuration: " ${HIKALI_CLANG_DEBUG_COMPILE_OPTIONS})
    endif()

    if("${TARGET_CPU}" STREQUAL "x86_64")
        # 启用AVX2
        set(HIKALI_CLANG_RELEASE_COMPILE_OPTIONS "-mavx2")
    endif()
    set(HIKALI_CLANG_RELEASE_COMPILE_OPTIONS ${HIKALI_CLANG_RELEASE_COMPILE_OPTIONS} CACHE STRING "Additional Clang compile options for release configurations")
    if(HIKALI_CLANG_RELEASE_COMPILE_OPTIONS)
        message("Additional Clang compile options for release configurations: " ${HIKALI_CLANG_RELEASE_COMPILE_OPTIONS})
    endif()

    target_compile_options(BuildSettings INTERFACE
        ${HIKALI_CLANG_COMPILE_OPTIONS}
        # 一些额外的警告
        -Wall -Wextra -Wuninitialized -Wconditional-uninitialized -Wextra-tokens -Wpointer-arith -Wloop-analysis -Wunused
        # 禁用部分警告
        -Wno-overloaded-virtual -Wno-incompatible-pointer-types-discards-qualifiers -Wno-unknown-pragmas
        -Wno-zero-as-null-pointer-constant -Wno-unused-parameter -Wno-invalid-offsetof
    )
    target_compile_options(BuildSettings INTERFACE "$<$<CONFIG:DEBUG>:${HIKALI_CLANG_DEBUG_COMPILE_OPTIONS}>")

    set(CLANG_RELEASE_OPTIONS -Wno-unused-variable ${HIKALI_CLANG_RELEASE_COMPILE_OPTIONS})
    target_compile_options(BuildSettings INTERFACE "$<$<NOT:$<CONFIG:Debug>>:${CLANG_RELEASE_OPTIONS}>")

    if(PLATFORM_WIN32)
        target_compile_options(BuildSettings
        INTERFACE
            -Wno-microsoft-exception-spec
            -Wno-tautological-constant-out-of-range-compare
        )
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "9.0")
        message(WARNING "gcc 9.0 and above seemingly produce invalid binary code with O2 and O3 optimization levels. Optimization in release configurations will be downgraded to O1. It is strongly recommended to use clang.")
        set(GCC_OPTIMIZATION_LEVEL "-O1")
        target_compile_options(BuildSettings INTERFACE $<$<CONFIG:Release,RelWithDebInfo>:${GCC_OPTIMIZATION_LEVEL}>)
    endif()
endif()

if(MSVC)
    option(HIKALI_INSTALL_PDB "Install PDB files" OFF)
else()
    set(HIKALI_INSTALL_PDB OFF)
endif()

file(RELATIVE_PATH HIKALI_RENDERER_DIR "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
set(HIKALI_RENDERER_DIR ${HIKALI_RENDERER_DIR} CACHE INTERNAL "Hikali renderer installation directory")

set(HIKALI_RENDERER_INSTALL_LIBS_LIST "" CACHE INTERNAL "Renderer libraries installation list")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation path" FORCE)
elseif(NOT IS_ABSOLUTE ${CMAKE_INSTALL_PREFIX})
    # CMAKE_INSTALL_PREFIX一定为绝对路径，否则rpath不能正常工作
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}" CACHE PATH "Installation path" FORCE)
    message("Transformed CMAKE_INSTALL_PREFIX into absolute path: " ${CMAKE_INSTALL_PREFIX})
endif()

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    # 寻找可用的.NET SDKs
    execute_process(COMMAND dotnet --list-sdks
                    RESULTS_VARIABLE RES
                    OUTPUT_VARIABLE DOT_NET_SDK_LIST)
    if(${RES} EQUAL 0)
        # 提取出list中列举的版本
        string(REGEX MATCHALL "[0-9]+\\.[0-9]+\\.[0-9]+" DOT_NET_SDK_VERSIONS "${DOT_NET_SDK_LIST}")
        set(DOT_NET_SDK_VERSIONS ${DOT_NET_SDK_VERSIONS} CACHE STRING "Available .NET SDK versions" FORCE)
        message("Available .NET SDK versions: ${DOT_NET_SDK_VERSIONS}")

        set(MAX_DOT_NET_SDK_VERSION "")
        foreach(VER IN LISTS DOT_NET_SDK_VERSIONS)
            if(VER VERSION_GREATER MAX_DOT_NET_SDK_VERSION)
                set(MAX_DOT_NET_SDK_VERSION "${VER}")
            endif()
        endforeach()
        set(MAX_DOT_NET_SDK_VERSION ${MAX_DOT_NET_SDK_VERSION} CACHE STRING "Maximum .NET SDK version" FORCE)
        message("  Maximum .NET SDK version: ${MAX_DOT_NET_SDK_VERSION}")
    else()
        message("dotnet command is not found. This indicates that .NET SDK is not installed on this machine.")
    endif()
endif()

option(HIKALI_BUILD_WIN32_GUI_AS_CONSOLE "Build Windows GUI applications using the console subsystem" OFF)

add_subdirectory(BuildTools)
add_subdirectory(Main)

include(BuildTools/CMake/BuildUtils.cmake)

install_combined_static_lib(
    "${CMAKE_STATIC_LIBRARY_PREFIX}RendererCore${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${HIKALI_RENDERER_INSTALL_LIBS_LIST}"
    RendererCore-static                 # Custom target name
    Renderer                            # Folder
    "${CMAKE_INSTALL_LIBDIR}/${HIKALI_RENDERER_DIR}/$<CONFIG>"    # Install destination
)