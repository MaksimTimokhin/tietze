cmake_minimum_required(VERSION 3.14)
project(Tietze)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_FLAGS -g)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(tietze main.cpp)
