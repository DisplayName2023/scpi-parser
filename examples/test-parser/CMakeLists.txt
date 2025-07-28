cmake_minimum_required(VERSION 3.28)

project(scpi-test-parser LANGUAGES C)

add_executable(scpi-test-parser main.c)

target_link_libraries(scpi-test-parser PRIVATE scpi scpi-common-example)

install(TARGETS scpi-test-parser
    RUNTIME DESTINATION bin
)

# Copy scpi DLLs to the output folder after build (Windows only)
if(WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:scpi>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()
