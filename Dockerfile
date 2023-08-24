# Use an official CMake base image
FROM ubuntu:latest

WORKDIR /app

COPY . .

# Install necessary dependencies
RUN apt-get update && apt-get install -y clang cmake build-essential

# Build project using CMake
RUN cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++
RUN cmake --build build --config Release
RUN cd build && make
