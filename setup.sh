#!/bin/bash
set -e

sudo apt update -y
rosdep update --rosdistro humble
rosdep install -i --from-paths src --rosdistro humble
