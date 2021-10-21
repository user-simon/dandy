cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(dandy)
add_library(dandy INTERFACE include/dandy/dandy.h)

add_subdirectory(tests)
