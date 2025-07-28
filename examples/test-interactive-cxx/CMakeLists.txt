cmake_minimum_required(VERSION 3.28)

project(scpi-test-interactive-cxx LANGUAGES CXX)

add_executable(scpi-test-interactive-cxx main.cpp)

target_link_libraries(scpi-test-interactive-cxx PRIVATE scpi scpi-common-cxx-example)

install(TARGETS scpi-test-interactive-cxx
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

