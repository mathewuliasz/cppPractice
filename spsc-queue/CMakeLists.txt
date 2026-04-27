cmake_minimum_required(VERSION 3.20)
project(SPSCQueue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Header-only library
add_library(spsc_queue INTERFACE)
target_include_directories(spsc_queue INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

# Main executable
add_executable(spsc_demo src/main.cpp)
target_link_libraries(spsc_demo PRIVATE spsc_queue)
target_compile_options(spsc_demo PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion
)

add_executable(spsc_tests tests/spsc_tests.cpp)
target_link_libraries(spsc_tests PRIVATE spsc_queue gtest_main)
target_compile_options(spsc_tests PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion
)