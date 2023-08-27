cmake_minimum_required(VERSION 3.16)
project(magnet)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_GENERATOR MATCHES "Ninja")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/magnet/Binaries/${CMAKE_BUILD_TYPE}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/magnet/Binaries/${CMAKE_BUILD_TYPE}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/magnet/Binaries/${CMAKE_BUILD_TYPE}")
else ()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/magnet/Binaries")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/magnet/Binaries")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/magnet/Binaries")
endif ()

set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Disable yaml-cpp build tools")
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Disable yaml-cpp tests")
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "Disable yaml-cpp contrib in library")
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

include_directories("magnet/Source")

add_subdirectory("magnet/Source")
add_subdirectory("magnet/ThirdParty")

target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-Werror>")
endif ()

if (MSVC)
    SET(CMAKE_CXX_FLAGS "/EHsc")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT magnet)
endif ()
