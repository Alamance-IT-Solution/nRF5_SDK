#!/bin/bash

# Specify the source directory
SOURCE_DIR="$HOME/arm-gcc-toolchain/bin/"

# Specify the target directory
TARGET_DIR="/usr/local/gcc-arm-none-eabi-7-2018-q2-update/bin"

# Create symbolic links for all files in the source directory
for file in "$SOURCE_DIR"/*; do
    if [ -f "$file" ]; then
        filename=$(basename "$file")
        sudo ln -s "$SOURCE_DIR/$filename" "$TARGET_DIR/$filename"
    fi
done
