cmake_minimum_required(VERSION 3.10)
project(Weather_Station_Dashboard)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "build test suite" OFF)

include_directories(
        /usr/include
       )

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
target_link_libraries(Weather_Station_Dashboard
)

# --- Tests ---
if(BUILD_TESTS)
    enable_testing()

    find_library(MOSQUITTO_LIBRARY mosquitto)
    find_path(MOSQUITTO_INCLUDE_DIR mosquitto.h)

    if (MOSQUITTO_LIBRARY AND MOSQUITTO_INCLUDE_DIR)
        message(STATUS "Found Mosquitto: ${MOSQUITTO_LIBRARY}")
        set(MOSQUITTO_FOUND TRUE)
    else()
        message(WARNING "Mosquitto not found - integration tests disabled")
        set(MOSQUITTO_FOUND FALSE)
    endif()

    add_executable(runTests
            src/weather_data.h
            tests/tests.cpp
    )
    target_include_directories(runTests PRIVATE
            ${CMAKE_SOURCE_DIR}/external/Crow/include
            )
    target_link_libraries(runTests
            PRIVATE
            GTest::gtest
            GTest::gtest_main
            GTest::gmock
            GTest::gmock_main
    )
    # Only add Mosquitto if found (for integration tests)
    if(MOSQUITTO_FOUND)
        target_include_directories(runTests PRIVATE ${MOSQUITTO_INCLUDE_DIR})
        target_link_libraries(runTests PRIVATE ${MOSQUITTO_LIBRARY})
        target_compile_definitions(runTests PRIVATE MQTT_ENABLED=1)
    endif()

    include(GoogleTest)
    gtest_discover_tests(runTests)
endif()
