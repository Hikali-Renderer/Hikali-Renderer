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