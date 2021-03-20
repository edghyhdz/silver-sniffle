cmake_minimum_required(VERSION 3.11.3)

# set(CMAKE_CXX_STANDARD 17)
project(silver_sniffle)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -lcurses -lform -pthread -lcrypto -lssl")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/executable)
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

find_package(CURL REQUIRED) 
find_package(OpenSSL REQUIRED)
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

# Set path where certificates will be located
set(CERTIFICATES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/executable/certificates/)

# Pass CERTIFICATES_PATH variable to SetPath.cmake
add_custom_command(TARGET silver_sniffle PRE_BUILD 
        COMMAND ${CMAKE_COMMAND} 
        -DCERTIFICATES_PATH=${CERTIFICATES_PATH} 
        -P ${CMAKE_CURRENT_SOURCE_DIR}/SetPath.cmake)


# sh file to run server from command line
FILE(WRITE ${CMAKE_BINARY_DIR}/runSilverServer.sh
"#!/bin/sh\n"
"${CMAKE_BINARY_DIR}/silver_server\n"
)

# sh file to run client from command line
FILE(WRITE ${CMAKE_BINARY_DIR}/runSilverSniffle.sh
"#!/bin/sh\n"
"arg1=\"$1\"\n"
"shift 1\n"
"${CMAKE_BINARY_DIR}/executable/silver_sniffle \"$@\"\n"
)

# Add script to execute sever CMakeList.txt
FILE(WRITE ${CMAKE_BINARY_DIR}/install_server.sh
"#!/bin/sh\n"
"rm ${CMAKE_BINARY_DIR}/CMakeCache.txt\n"
"/usr/local/bin/cmake ${CMAKE_SOURCE_DIR}/server\n"
"make\n"
)

# Add script to execute sever CMakeList.txt
FILE(WRITE ${CMAKE_BINARY_DIR}/install_client.sh
"#!/bin/sh\n"
"rm ${CMAKE_BINARY_DIR}/CMakeCache.txt\n"
"/usr/local/bin/cmake ${CMAKE_SOURCE_DIR}\n"
"make\n"
)

FILE(WRITE ${CMAKE_BINARY_DIR}/install.sh
"#!/bin/sh\n"
"source ${CMAKE_BINARY_DIR}/install_server.sh\n"
"source ${CMAKE_BINARY_DIR}/install_client.sh\n"
"source ${CMAKE_BINARY_DIR}/install_client.sh\n"
"sudo mv ${CMAKE_BINARY_DIR}/runSilverServer.sh /usr/local/bin/silver_server\n" 
"sudo chown root: /usr/local/bin/silver_server\n"
"sudo chmod 755 /usr/local/bin/silver_server\n"
"sudo mv ${CMAKE_BINARY_DIR}/runSilverSniffle.sh /usr/local/bin/silver_sniffle\n" 
"sudo chown root: /usr/local/bin/silver_sniffle\n"
"sudo chmod 755 /usr/local/bin/silver_sniffle\n"
)

set_property(TARGET silver_sniffle PROPERTY CXX_STANDARD 17)
target_link_libraries(silver_sniffle OpenSSL::SSL)
target_link_libraries(silver_sniffle ${CURSES_LIBRARIES})