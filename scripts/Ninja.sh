#!/bin/zsh

# This script is used to build the project using the Ninja generator.
cd ..
cmake -S . -B magnet/Build -G "Ninja"
cmake --build magnet/Build