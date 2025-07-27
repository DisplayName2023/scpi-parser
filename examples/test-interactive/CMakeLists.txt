cmake_minimum_required(VERSION 3.28)

project(scpi-test-interactive LANGUAGES C)

add_executable(scpi-test-interactive main.c)

target_link_libraries(scpi-test-interactive PRIVATE scpi scpi-common-example)

install(TARGETS scpi-test-interactive
    RUNTIME DESTINATION bin
)



add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}> 
    
    
)