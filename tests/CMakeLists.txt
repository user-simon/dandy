cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(googletest)

project(tests)
add_executable(tests common.h traits.cpp constructors.cpp expressions.cpp math.cpp conversions.cpp serialization.cpp)
include_directories(tests ../include googletest/googletest/include)
target_link_libraries(tests PUBLIC gtest_main)
