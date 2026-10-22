#!/bin/bash
set -e

# clean
rm -rf build install log

# Set the default build type
BUILD_TYPE=RelWithDebInfo
colcon build \
        --merge-install \
        --symlink-install \
        --cmake-args "-DCMAKE_BUILD_TYPE=$BUILD_TYPE" \
        -Wall -Wextra -Wpedantic