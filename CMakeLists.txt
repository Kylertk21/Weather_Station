cmake_minimum_required(VERSION 3.10)
project(Weather_Station_Dashboard)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Crow (if installed system-wide)
find_package(Crow REQUIRED)

# Define the executable — this is your target name
add_executable(Weather_Station_Dashboard src/main.cpp)

# Link Crow to the SAME target name
target_link_libraries(Weather_Station_Dashboard PRIVATE /usr/include)
