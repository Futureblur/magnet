cmake_minimum_required(VERSION 3.16)
project(magnet)

set(CMAKE_CXX_STANDARD 17)

add_executable(magnet EntryPoint.cpp Application.h Application.cpp CommandHandler.h CommandHandler.cpp
        Core.h
        Project.h
        Project.cpp
        CmakeEmitter.h
        CmakeEmitter.cpp
        Platform/Platform.h
        Platform/macOSPlatform.cpp
        Platform/WindowsPlatform.cpp)

# Set rpath relative to app
if (NOT MSVC)
    set_target_properties(magnet PROPERTIES LINK_FLAGS "-Wl,-rpath,./")
else ()
    set_target_properties(magnet PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/Binaries/Debug")
endif ()

# Precompiled headers
target_precompile_headers(magnet PUBLIC PCH.h)

target_link_libraries(magnet yaml-cpp)