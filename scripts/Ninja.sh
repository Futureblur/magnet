#!/bin/zsh

# This script is used to build the project using the Ninja generator.
cd ..
cmake -S . -B magnet/Build -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++
cmake --build magnet/Build