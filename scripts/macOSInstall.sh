#!/bin/bash

# Get the current directory
current_dir=$(pwd)

# Path to the magnet binary
binary_path="$current_dir/../magnet/Binaries/magnet"

# Create an alias
sudo ln -s $binary_path /usr/local/bin

echo "Alias 'magnet' has been created."
