#!/bin/bash

sudo apt update

# Get first argument
if [ -z "$1" ]
then
    echo "No argument supplied. Please specify the generator you want to use."
    echo "Usage: ./Linux.sh [--ninja | --make]"
    exit 1
fi

# Check if the generator is valid
if [ "$1" != "--ninja" ] && [ "$1" != "--make" ]
then
    echo "Invalid generator."
    echo "Usage: ./Linux.sh [--ninja | --make]"
    exit 1
fi

# Install CMake if it is not installed
if ! command -v cmake &> /dev/null
then
    echo "CMake could not be found. Installing CMake..."
    sudo apt install cmake
fi

# Remove old build files
if [ -d "../magnet/Build" ]; then
    rm -rf ../magnet/Build
fi

# Remove old binaries
if [ -d "../magnet/Binaries" ]; then
    rm -rf ../magnet/Binaries
fi

# Run generator according to the first argument
if [ "$1" == "--ninja" ]
then
    if ! command -v ninja &> /dev/null
    then
        echo "Ninja could not be found. Installing Ninja..."
        sudo apt install ninja-build
    fi

    echo "Running Ninja generator..."
    chmod +x ./Ninja.sh
    ./Ninja.sh
fi

if [ "$1" == "--make" ]
then
    echo "Running Makefile generator..."
    chmod +x ./Makefile.sh
    ./Makefile.sh
fi

# Get the current directory
current_dir=$(pwd)

# Path to the magnet binary
# Check if Debug folder exists
if [ -f "$current_dir/../magnet/Binaries/Debug/magnet" ]; then
    binary_path="$current_dir/../magnet/Binaries/Debug/magnet"
else
    binary_path="$current_dir/../magnet/Binaries/magnet"
fi

# Remove old symlink if it exists
if [ -f "/usr/local/bin/magnet" ]; then
    sudo rm /usr/local/bin/magnet
fi

# Create symlink
sudo ln -s $binary_path /usr/local/bin/magnet

echo "You can now run 'magnet' from anywhere in the terminal."
echo "To uninstall, run 'sudo rm -f /usr/local/bin/magnet'."

