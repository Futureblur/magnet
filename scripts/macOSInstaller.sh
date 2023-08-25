#!/bin/bash

# Get first argument
if [ -z "$1" ]
then
    echo "No argument supplied. Please specify the generator you want to use."
    echo "Usage: ./macOSInstaller.sh [--ninja | --xcode]"
    exit 1
fi

# Check if the generator is valid
if [ "$1" != "--ninja" ] && [ "$1" != "--xcode" ]
then
    echo "Invalid generator."
    echo "Usage: ./macOSInstaller.sh [--ninja | --xcode]"
    exit 1
fi

# Install CMake using brew if it is not installed
if ! command -v cmake &> /dev/null
then
    echo "CMake could not be found. Installing CMake using brew..."
    brew install cmake
fi

echo "Removing old build files and binaries..."

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
    # Install Ninja using brew if it is not installed
    if ! command -v ninja &> /dev/null
    then
        echo "Ninja could not be found. Installing Ninja using brew..."
        brew install ninja
    fi

    echo "Running Ninja generator..."
    chmod +x ./Ninja.sh
    ./Ninja.sh
fi

if [ "$1" == "--xcode" ]
then
    echo "Running Xcode generator..."
    chmod +x ./Xcode.sh
    ./Xcode.sh
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

# Remove old alias if it exists
echo "Removing old alias..."
sudo rm -f /usr/local/bin/magnet

# Create an alias
echo "Creating alias..."
sudo ln -s $binary_path /usr/local/bin

echo "You can now run 'magnet' from anywhere in the terminal."
echo "To uninstall, run 'sudo rm -f /usr/local/bin/magnet'."
