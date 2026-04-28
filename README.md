## Initial Connection to Nano
Getting our local docker containers to communicate with the Nano requires some network hopping initially. This is just so that VSCode can install its server to interface with everything. All subsequent connections do not require this.

Steps to connect initially are:
- connect to wifi build your container and tag it as ros2 to make it easier to reference by running this in .devcontainer:
`docker build --tag ros2 .`
- connect to the network, and run this while in the root directory of the project: <br>
`docker run --net=host --cap-add=SYS_PTRACE --security-opt=seccomp:unconfined --security-opt=apparmor:unconfined --volume=/tmp/.X11-unix:/tmp/.X11-unix --volume .:/lunabot/ --device=/dev/input --ipc=host -dit ros2` <br>
If you are on Linux, and this if you are on Windows: <br>
`docker run --net=host --cap-add=SYS_PTRACE --security-opt=seccomp:unconfined --security-opt=apparmor:unconfined --volume=/tmp/.X11-unix:/tmp/.X11-unix --volume .:/lunabot/ --volume=/mnt/wslg:/mnt/wslg --ipc=host -dit ros2`
- connect to wifi again, open the docker panel on the side, right click the ros2 container, and click Attach Visual Studio Code:<br>
![Screenshot_20241003_194203](https://github.com/user-attachments/assets/399356a0-c7f8-4388-b973-f1f1c64b5be0) <br>
- now, go back to the network, and, in the container, run `export ROS_DOMAIN_ID=1` <br>
You should now be able to communicate with the Nano from your docker container


For all subsequent connections, you just have to connect to the network and run `docker container start <container_name>` with the automatically assigned name of the container. In my case, mine was named vigorous_benz, so I would run `docker container start vigorous_benz`, and then I can communicate with the Nano after setting my ROS_DOMAIN_ID=1
