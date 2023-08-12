#!/bin/zsh

# This script is used to build the project using the Xcode generator.
cd ..
cmake -S . -B magnet/Build -G "Xcode"
cmake --build magnet/Build