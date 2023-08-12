#!/bin/zsh

# This script is used to build the project using the Visual Studio 2022 generator.
cd ..
cmake -S . -B magnet/Build -G "Visual Studio 17 2022" -A x64
cmake --build magnet/Build