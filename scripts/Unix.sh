#!/bin/zsh

# This script is used to build the project on Unix systems.
cd ..
cmake -S . -B magnet/Build -G "Unix Makefiles"
cmake --build magnet/Build
