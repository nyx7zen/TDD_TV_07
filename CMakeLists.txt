cmake_minimum_required(VERSION 3.14)
project(TDD_TV CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    TLS_VERIFY OFF
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Library
add_library(tv_controller src/TVChannelController.cpp)
target_include_directories(tv_controller PUBLIC include)

# Tests
enable_testing()

add_executable(fake_tuner_test test/FakeTunerTest.cpp)
target_link_libraries(fake_tuner_test tv_controller GTest::gtest_main)

add_executable(controller_test test/TVChannelControllerTest.cpp)
target_link_libraries(controller_test tv_controller GTest::gtest_main)

add_executable(controller_mock_test test/TVChannelControllerMockTest.cpp)
target_link_libraries(controller_mock_test tv_controller GTest::gtest_main GTest::gmock_main)

include(GoogleTest)
# Windows: gtest_discover_tests per-case filters break on non-ASCII test names
# (CTest passes mangled --gtest_filter → 0 tests run, false Pass). Register whole executables.
if(WIN32)
    add_test(NAME fake_tuner_test COMMAND fake_tuner_test)
    add_test(NAME controller_test COMMAND controller_test)
    add_test(NAME controller_mock_test COMMAND controller_mock_test)
else()
    gtest_discover_tests(fake_tuner_test)
    gtest_discover_tests(controller_test)
    gtest_discover_tests(controller_mock_test)
endif()