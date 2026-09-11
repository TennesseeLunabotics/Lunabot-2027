#!/bin/bash

# Check if the interface exists
if ! ip link show can0 &> /dev/null; then
    echo "Interface can0 does not exist."
    echo "If you have a physical CAN adapter, make sure it's connected."
    echo "If you want to create a virtual CAN interface for testing, use:"
    echo "sudo ip link add dev $interface type vcan"
    exit 1
fi

# Try to set the interface type and bitrate
if sudo ip link set can0 type can bitrate 1000000; then
    echo "Set can0 type to CAN with 1Mbps bitrate."
else
    echo "Failed to set can0 type and bitrate."
    echo "If this is a virtual CAN interface, you can skip this step."
fi

# Bring up the interface
if sudo ip link set can0 up; then
    echo "Brought up can0."
else
    echo "Failed to bring up can0."
    exit 1
fi

echo "CAN interface can0 setup complete."
