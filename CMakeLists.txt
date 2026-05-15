cmake_minimum_required(VERSION 3.14)
project(TDD_TV CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Coverage 옵션
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    TLS_VERIFY OFF
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
    ApprovalTests
    GIT_REPOSITORY https://github.com/approvals/ApprovalTests.cpp.git
    GIT_TAG v.10.13.0
)
FetchContent_MakeAvailable(ApprovalTests)

include(GoogleTest)

# Library
add_library(tv_controller src/TVChannelController.cpp)
target_include_directories(tv_controller PUBLIC include)

# Tests
enable_testing()

add_executable(fake_tuner_test test/FakeTunerTest.cpp)
target_link_libraries(fake_tuner_test tv_controller GTest::gtest_main)
gtest_discover_tests(fake_tuner_test)

add_executable(controller_test test/TVChannelControllerTest.cpp)
target_link_libraries(controller_test tv_controller GTest::gtest_main)
gtest_discover_tests(controller_test)

add_executable(controller_mock_test test/TVChannelControllerMockTest.cpp)
target_link_libraries(controller_mock_test tv_controller GTest::gtest_main GTest::gmock_main)
gtest_discover_tests(controller_mock_test)

add_executable(approval_test
    test/ApprovalTestsMain.cpp
    test/ApprovalTest.cpp
)
target_link_libraries(approval_test
    tv_controller
    ApprovalTests::ApprovalTests
    GTest::gtest
)
gtest_discover_tests(approval_test
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DISCOVERY_MODE PRE_TEST
)