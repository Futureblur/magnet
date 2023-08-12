cmake_minimum_required(VERSION 3.16)
project(magnet)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/magnet/Binaries")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/magnet/Binaries")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/magnet/Binaries")

include_directories("magnet/Source")

add_subdirectory("magnet/Source")
add_subdirectory("magnet/ThirdParty")