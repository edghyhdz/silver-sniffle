cmake_minimum_required(VERSION 3.11.3)

# set(CMAKE_CXX_STANDARD 17)
project(silver_server)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Find all executables
file(GLOB project_SRCS *.cpp) #src/*.h

# Include utils
include_directories(include)
include_directories(../utils)

# Add project executable
add_executable(silver_server ${project_SRCS})

set_property(TARGET silver_server PROPERTY CXX_STANDARD 17)
