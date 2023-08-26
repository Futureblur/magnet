#!/bin/bash

# This script is used to build the project using the Ninja generator.
cd ..
cmake -S . -B magnet/Build -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
cmake --build magnet/Build