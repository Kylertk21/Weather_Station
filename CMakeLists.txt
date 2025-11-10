cmake_minimum_required(VERSION 3.10)
project(Weather_Station_Dashboard)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Crow includes ---
include_directories(external/Crow/include)
include_directories(external/asio/asio/include)

# --- Main executable ---
add_executable(Weather_Station_Dashboard src/main.cpp)

# --- GoogleTest via FetchContent ---
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- Testing ---
enable_testing()

add_executable(runTests tests/tests.cpp)

target_link_libraries(runTests
        PRIVATE
        GTest::gtest
        GTest::gtest_main
        pthread
)

include(GoogleTest)
gtest_discover_tests(runTests)
