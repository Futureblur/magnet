# Use an official CMake base image
FROM ubuntu:latest

WORKDIR /app

COPY . .

# Install necessary dependencies
RUN apt-get update && apt-get install -y clang cmake build-essential ninja-build

# Build project using CMake
RUN cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -G Ninja
RUN cmake --build build
RUN cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release -G Ninja
RUN cmake --build build
