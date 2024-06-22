cmake_minimum_required (VERSION 3.10)

project(simple_tcp_network)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

find_package(Boost 1.40.0 REQUIRED system)

add_subdirectory(apps)
add_subdirectory(libs)
