cmake_minimum_required(VERSION 3.16)
project(magnet)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory("yaml-cpp")

target_include_directories(magnet PUBLIC "yaml-cpp/include")