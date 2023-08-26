#!/bin/bash

echo "Updating apt..."
sudo apt update

echo "Installing dependencies..."
sudo apt install cmake ninja-build clang

# Remove old build files
if [ -d "../magnet/Build" ]; then
    rm -rf ../magnet/Build
fi

# Remove old binaries
if [ -d "../magnet/Binaries" ]; then
    rm -rf ../magnet/Binaries
fi

echo "Running Ninja generator..."
chmod +x ./Ninja.sh
./Ninja.sh

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

