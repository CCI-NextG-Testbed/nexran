
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Source tree builds are disabled.")
endif()

cmake_minimum_required(VERSION 3.4)
project(NexRANApp)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

option(ENABLE_BUILD_TIMESTAMP "Enable build timestamp" OFF)

include(buildinfo)

find_package(Threads REQUIRED)
find_package(Pistache 0.0.2 REQUIRED)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(${PROJECT_SOURCE_DIR}/include)

#add_subdirectory(lib)
add_subdirectory(src)
