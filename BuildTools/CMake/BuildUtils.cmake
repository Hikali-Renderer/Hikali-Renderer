function(install_combined_static_lib COMBINED_LIB_NAME LIBS_LIST CUSTOM_TARGET_NAME CUSTOM_TARGET_FOLDER INSTALL_DESTINATION)

    foreach(LIB ${LIBS_LIST})
        list(APPEND COMBINED_LIB_TARGET_FILES $<TARGET_FILE:${LIB}>)
    endforeach(LIB)

    if(MSVC)
        add_custom_command(
            OUTPUT "$<CONFIG>/${COMBINED_LIB_NAME}"
            COMMAND lib.exe /OUT:"$<CONFIG>/${COMBINED_LIB_NAME}" ${COMBINED_LIB_TARGET_FILES}
            DEPENDS ${LIBS_LIST}
            COMMENT "Combining libraries..."
        )
        add_custom_target(${CUSTOM_TARGET_NAME} ALL DEPENDS "$<CONFIG>/${COMBINED_LIB_NAME}")
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${COMBINED_LIB_NAME}"
                DESTINATION ${INSTALL_DESTINATION}
        )
    else()

        if(PLATFORM_WIN32)
            # do NOT use stock ar on MinGW
            find_program(AR NAMES x86_64-w64-mingw32-gcc-ar)
        else()
            set(AR ${CMAKE_AR})
        endif()

        if(AR)
            add_custom_command(
                OUTPUT ${COMBINED_LIB_NAME}
                # Delete all object files from current directory
                COMMAND ${CMAKE_COMMAND} -E remove "*${CMAKE_C_OUTPUT_EXTENSION}"
                DEPENDS ${LIBS_LIST}
                COMMENT "Combining libraries..."
            )

            # Unpack all object files from all targets to current directory
            foreach(LIB_TARGET ${COMBINED_LIB_TARGET_FILES})
                add_custom_command(
                    OUTPUT ${COMBINED_LIB_NAME}
                    COMMAND ${AR} -x ${LIB_TARGET}
                    APPEND
                )
            endforeach()

            # Pack object files to a combined library and delete them
            add_custom_command(
                OUTPUT ${COMBINED_LIB_NAME}
                COMMAND ${AR} -crs ${COMBINED_LIB_NAME} "*${CMAKE_C_OUTPUT_EXTENSION}"
                COMMAND ${CMAKE_COMMAND} -E remove "*${CMAKE_C_OUTPUT_EXTENSION}"
                APPEND
            )

            add_custom_target(${CUSTOM_TARGET_NAME} ALL DEPENDS ${COMBINED_LIB_NAME})
            install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${COMBINED_LIB_NAME}"
                    DESTINATION ${INSTALL_DESTINATION}
            )
        else()
            message("ar command is not found")
        endif()
    endif()

    if(TARGET ${CUSTOM_TARGET_NAME})
        set_target_properties(${CUSTOM_TARGET_NAME} PROPERTIES
            FOLDER ${CUSTOM_TARGET_FOLDER}
        )
    else()
        message("Unable to find librarian tool. Combined ${COMBINED_LIB_NAME} static library will not be produced.")
    endif()

endfunction()

# Returns path to the target relative to CMake root
function(get_target_relative_dir _TARGET _DIR)
    get_target_property(TARGET_SOURCE_DIR ${_TARGET} SOURCE_DIR)
    file(RELATIVE_PATH TARGET_RELATIVE_PATH "${CMAKE_SOURCE_DIR}" "${TARGET_SOURCE_DIR}")
    set(${_DIR} ${TARGET_RELATIVE_PATH} PARENT_SCOPE)
endfunction()

function(set_common_target_properties TARGET)

    # Check if a second argument (MIN_CXX_STANDARD) is provided
    if(ARGC GREATER 1)
        set(MIN_CXX_STANDARD ${ARGV1})
    endif()

    get_target_property(TARGET_TYPE ${TARGET} TYPE)

    set(CXX_STANDARD 17)
    if(MIN_CXX_STANDARD)
        if(MIN_CXX_STANDARD GREATER ${CXX_STANDARD})
            set(CXX_STANDARD ${MIN_CXX_STANDARD})
        endif()
    endif()

    set_target_properties(${TARGET} PROPERTIES
        # It is crucial to set CXX_STANDARD flag to only affect c++ files and avoid failures compiling c-files:
        # error: invalid argument '-std=c++17' not allowed with 'C/ObjC'
        CXX_STANDARD ${CXX_STANDARD}
        CXX_STANDARD_REQUIRED ON
    )

    if(MSVC)
        # For msvc, enable link-time code generation for release builds (I was not able to
        # find any way to set these settings through interface library BuildSettings)
        if(TARGET_TYPE STREQUAL STATIC_LIBRARY)
            foreach(REL_CONFIG ${RELEASE_CONFIGURATIONS})
                set_target_properties(${TARGET} PROPERTIES
                    STATIC_LIBRARY_FLAGS_${REL_CONFIG} /LTCG
                )
            endforeach()
        else()
            foreach(REL_CONFIG ${RELEASE_CONFIGURATIONS})
                set_target_properties(${TARGET} PROPERTIES
                    LINK_FLAGS_${REL_CONFIG} "/LTCG /OPT:REF /INCREMENTAL:NO"
                )
            endforeach()
        endif()
    else()
        set_target_properties(${TARGET} PROPERTIES
            CXX_VISIBILITY_PRESET hidden # -fvisibility=hidden
            C_VISIBILITY_PRESET hidden # -fvisibility=hidden
            VISIBILITY_INLINES_HIDDEN TRUE

            # Without -fPIC option GCC fails to link static libraries into dynamic library:
            #  -fPIC
            #      If supported for the target machine, emit position-independent code, suitable for
            #      dynamic linking and avoiding any limit on the size of the global offset table.
            POSITION_INDEPENDENT_CODE ON

            C_STANDARD 11 # MSVC requires legacy C standard
        )

        if(NOT MINGW_BUILD)
            # Do not disable extensions when building with MinGW!
            set_target_properties(${TARGET} PROPERTIES
                CXX_EXTENSIONS OFF
            )
        endif()

        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND TARGET_TYPE STREQUAL SHARED_LIBRARY)
            target_link_options(${TARGET}
            PRIVATE
                # Disallow missing direct and indirect dependencies to ensure that .so is self-contained
                LINKER:--no-undefined
                LINKER:--no-allow-shlib-undefined
            )
        endif()
    endif() # if(MSVC)
endfunction()

# Performs installation steps for the core library
function(install_core_lib _TARGET)
    get_target_relative_dir(${_TARGET} TARGET_RELATIVE_PATH)

    get_target_property(TARGET_TYPE ${_TARGET} TYPE)
    if(TARGET_TYPE STREQUAL STATIC_LIBRARY)
        list(APPEND HIKALI_RENDERER_INSTALL_LIBS_LIST ${_TARGET})
        set(HIKALI_RENDERER_INSTALL_LIBS_LIST ${HIKALI_RENDERER_INSTALL_LIBS_LIST} CACHE INTERNAL "Core libraries installation list")
    elseif(TARGET_TYPE STREQUAL SHARED_LIBRARY)
        install(TARGETS                 ${_TARGET}
                ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${HIKALI_RENDERER_DIR}/$<CONFIG>"
                LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${HIKALI_RENDERER_DIR}/$<CONFIG>"
                RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${HIKALI_RENDERER_DIR}/$<CONFIG>"
        )
        if (HIKALI_INSTALL_PDB)
            install(FILES $<TARGET_PDB_FILE:${_TARGET}> DESTINATION "${CMAKE_INSTALL_BINDIR}/${HIKALI_RENDERER_DIR}/$<CONFIG>" OPTIONAL)
        endif()
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/interface")
        install(DIRECTORY    interface
                DESTINATION  "${CMAKE_INSTALL_INCLUDEDIR}/${TARGET_RELATIVE_PATH}/"
        )
    endif()
endfunction()