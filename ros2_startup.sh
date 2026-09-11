sudo ip link set can0 up type can bitrate 1000000
source /home/lunabot/lunabot/install/local_setup.sh
ros2 launch /home/lunabot/lunabot/launch/testlaunch.py
