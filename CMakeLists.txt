cmake_minimum_required(VERSION 3.10)
project(Weather_Station_Dashboard)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Include Crow headers
include_directories(external/Crow/include)
include_directories(external/Crow/include external/asio/asio/include)
include_directories(external/gtest/googletest/include)

# Add your sources
add_executable(Weather_Station_Dashboard
        src/main.cpp
)
