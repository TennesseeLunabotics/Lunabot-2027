# Lunabot-2027
Lunabotics repository for the 2026-2027 NASA Lunabotic compition.

## Container Setup
This documents how to setup and run our code inside a container using Docker or Podman. Containers keeps
ROS 2 and its dependencies isolated so your host system stays clean and not have bunch of software conflicts.
Unlike  a traditional virtual machine (VM), containers share the host kernel aka providing near native performance.

1. Create a workspace directory on your host machine to store the source code. This will be mounted diretly into the container.
```
mkdir ~/Documents/ros2
```
2. Run the following command below to start the container.
   (Also, You can replace `docker` with `podman` directly without changing anything else.
   Fedora and some other will have podman preinstalled.
```
docker run -it --rm \
  --privileged \
  --network=host \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v ~/Documents/ros2:/root/ros2_ws:Z \
  -w /root/ros2_ws \
  osrf/ros:humble-desktop \
  bash -c "export FASTDDS_BUILTIN_TRANSPORTS=UDPv4 && source /opt/ros/humble/setup.bash && exec bash"
```

## build
We have dedicated `build.sh` script that automatically removes old build artifacts and rebuilds from scratch.
You may need to make script executable first with `chmod +x build.sh`
