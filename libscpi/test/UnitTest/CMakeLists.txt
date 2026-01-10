# Google Test Unit Tests
# This CMakeLists.txt sets up Google Test for unit testing

# Find GTest package (provided by Conan)
find_package(GTest REQUIRED)

# Enable C++ for Google Test
enable_language(CXX)

# Include GoogleTest module for test discovery
include(GoogleTest)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

# Collect all test source files
file(GLOB TEST_SOURCES "*.cpp")

# Only create test executable if test sources exist
if(TEST_SOURCES)
    # Create test executable
    add_executable(scpi_parser_utest ${TEST_SOURCES})
    
    # Link against the scpi library and GTest
    target_link_libraries(scpi_parser_utest PRIVATE
        scpi
        GTest::gtest
        GTest::gtest_main
    )
    
    # On Windows, handle DLL dependencies
    if(WIN32)
        # Get GTest DLL directory - try multiple possible locations
        get_target_property(GTEST_GTEST_LOCATION GTest::gtest IMPORTED_LOCATION_RELEASE)
        if(NOT GTEST_GTEST_LOCATION)
            get_target_property(GTEST_GTEST_LOCATION GTest::gtest IMPORTED_LOCATION)
        endif()
        if(NOT GTEST_GTEST_LOCATION)
            get_target_property(GTEST_GTEST_LOCATION GTest::gtest IMPORTED_LOCATION_DEBUG)
        endif()
        
        # Get directory containing DLLs
        if(GTEST_GTEST_LOCATION)
            get_filename_component(GTEST_DLL_DIR "${GTEST_GTEST_LOCATION}" DIRECTORY)
        endif()
        
        # Copy GTest DLLs to output directory after build
        if(GTEST_DLL_DIR AND EXISTS "${GTEST_DLL_DIR}")
            file(GLOB GTEST_DLLS "${GTEST_DLL_DIR}/*.dll")
            foreach(dll ${GTEST_DLLS})
                get_filename_component(dll_name "${dll}" NAME)
                add_custom_command(TARGET scpi_parser_utest POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${dll}"
                        $<TARGET_FILE_DIR:scpi_parser_utest>/${dll_name}
                    COMMENT "Copying GTest DLL: ${dll_name}"
                )
            endforeach()
        endif()
        
        # Also copy scpi DLL if it's a shared library
        if(BUILD_SHARED_LIBS)
            add_custom_command(TARGET scpi_parser_utest POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:scpi>
                    $<TARGET_FILE_DIR:scpi_parser_utest>/$<TARGET_FILE_NAME:scpi>
                COMMENT "Copying scpi DLL"
            )
        endif()
    endif()
    
    # Register with CTest
    add_test(NAME scpi_parser_utest COMMAND scpi_parser_utest)
    
    # Enable test discovery
    # Note: On Windows, gtest_discover_tests runs during build/configure phase
    # which may fail if DLLs aren't in PATH. The DLLs are copied in POST_BUILD,
    # so discovery might fail. If it does, you can:
    # 1. Run tests manually: ctest --test-dir build
    # 2. Or run the executable directly: build/libscpi/test/UnitTest/Release/scpi_parser_utest.exe
    # 3. Or disable discovery by commenting out the gtest_discover_tests line below
    if(WIN32)
        # Option 1: Try discovery (may fail if DLLs not in PATH during configure)
        # gtest_discover_tests(scpi_parser_utest)
        
        # Option 2: Skip discovery during build - tests will still work when run manually
        # Tests can be run with: ctest or directly executing the test binary
        message(STATUS "Skipping automatic test discovery on Windows to avoid DLL path issues")
        message(STATUS "Run tests manually with: ctest --test-dir build")
        message(STATUS "Or execute: build/libscpi/test/UnitTest/Release/scpi_parser_utest.exe")
    else()
        gtest_discover_tests(scpi_parser_utest)
    endif()
endif()
