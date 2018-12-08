cmake_minimum_required(VERSION 3.0)

project (logflux)

add_subdirectory(tinyxml2)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(logflux)
