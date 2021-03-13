cmake_minimum_required(VERSION 3.11.3)

# set(CMAKE_CXX_STANDARD 17)
project(silver_sniffle)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -lcurses -lform -pthread")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/executable)
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

find_package(CURL REQUIRED) 
include_directories(${CURL_INCLUDE_DIR})

# Find all executables
file(GLOB project_SRCS src/*.cpp) #src/*.h

# Make directories to get certs from
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/executable/certificates)

# Include utils
include_directories(include)
include_directories(utils)

# Add project executable
add_executable(silver_sniffle ${project_SRCS})

set_property(TARGET silver_sniffle PROPERTY CXX_STANDARD 17)
target_link_libraries(silver_sniffle ${CURSES_LIBRARIES})