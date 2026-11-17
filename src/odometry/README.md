# odometry

Works out how far the Lunabot has moved and turned by blending three sensors, so no single one is fully trusted on its own.
This process of merging the data should mean we will always have reliable data for where the Lunabot is.
(Based off of Damien's "unitree_lidar" branch)

*****NOTE: There are a lot of constants and measurements required from the robot, 
fill these in when they are found (locations listed below).*****

Node - Job

`lidar_relay_node` - LiDAR point cloud -> same cloud in robot axes on `lidar/cloud_base`, and
LiDAR's built-in IMU -> bias-corrected gyro in robot axes on `imu/gyro`

`kiss_icp_node` ([KISS-ICP](https://github.com/PRBonn/kiss-icp)) - Lines up LiDAR scans -> robot motion on `kiss/odometry`

`odometry_node` - Drive encoders -> forward speed on `encoder/odom`

`ekf_node` (robot_localization) - Blends all three; publishes `odom -> base_link` and `/odometry/filtered`

Each source covers for the others. LiDAR/KISS-ICP is most accurate and doesn't slip. The IMU always
measures turning. Encoders always measure forward speed, but are prone to slipping.
All are combined to hopefully create a good position estimate.

Start everything with `ros2 launch odometry odometry.launch.py`.

## Before first run

1. *****Fill in the REQUIRED values at the top of `launch/odometry.launch.py`: the lidar mount
   (used for both the cloud and the gyro) and drive geometry. Each node refuses to start until
   they are valid.*****
2. Check the mount: turning counter-clockwise must give a positive `imu/gyro` z, and in RViz (fixed
   frame `base_link`) the wall in front of the robot must appear at +x on `lidar/cloud_base`.
3. Keep robot still for first few seconds, until log says "Gyro calibration done".
4. Drive a measured 3 m straight and one full spin in place, then compare
   `/odometry/filtered` with real motion. Record so values can be tuned.

## Integration notes (changes needed outside this package)

- KISS-ICP is built from source by `.devcontainer/Dockerfile`, because it
  is not packaged for Humble. `robot_localization` is installed by `setup.sh` (`rosdep install`),
  because this package's `package.xml` lists it.
- Only ONE publisher may provide `odom -> base_link`. The main launch
  (`lunabot_bringup/launch/launch.py`) runs this package with `use_odometry:=true`, which also
  turns off its fixed `odom -> base_link`; it defaults to false until the REQUIRED values are set.
  *****`lunabot_bringup/launch/lidar.launch.py` still always starts a fixed `odom -> base_link`, so
  don't run it together with this package.*****
- Don't use TF for lidar mount: driver's lidar frame moves with the IMU's orientation estimate,
  so the lidar relay uses measured mount values instead.

## Limits

- KISS-ICP never rejects a scan. Where LiDAR can't tell movement apart (like a long, flat
  corridor with nothing else), it can under-report motion and the EKF will believe it. The
  arena has walls pretty close on every side, so this shouldn't be an issue.

## Layout

- `include/odometry/params.hpp`: reads and checks the settings
- `src/`: the two nodes; `launch/`: all settings, and starts all four
