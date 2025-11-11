cmake_minimum_required(VERSION 3.10)
project(Weather_Station_Dashboard)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Crow includes ---
include_directories(${CMAKE_SOURCE_DIR}/external/Crow/include)

# --- GoogleTest via FetchContent ---
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- Main executable ---
add_executable(Weather_Station_Dashboard
        src/main.cpp
        src/weather_data.h
)

# --- Tests ---
if(BUILD_TESTS)
    enable_testing()
    add_executable(runTests
            tests/tests.cpp
            src/weather_data.h
    )
    target_include_directories(runTests PRIVATE ${CMAKE_SOURCE_DIR}/external/Crow/include)
    target_link_libraries(runTests PRIVATE GTest::gtest GTest::gtest_main pthread)
    include(GoogleTest)
    gtest_discover_tests(runTests)
endif()
