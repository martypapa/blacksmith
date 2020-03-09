cmake_minimum_required(VERSION 3.1)

project(tests)
set(CMAKE_CXX_STANDARD 17)

include_directories(include)

set(TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples.cpp)
add_executable(tests ${TEST_SOURCES})