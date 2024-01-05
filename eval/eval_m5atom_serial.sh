# directory名
directory=m5atom_serial
# directoryがなければ作成
mkdir -p $directory

# agentを起動
docker run --name microros-agent --rm \
    -v /dev:/dev \
    --privileged \
    --net=host \
    microros/micro-ros-agent:humble \
    serial --dev /dev/ttyUSB0 --baud 115200 &

# 5秒待機後、ROS2のノードを起動
sleep 5
source /opt/ros/humble/setup.bash
source ../ping_ros2/install/setup.bash
ros2 run ping_pong_measurement ping_pong_measurement $directory/
docker kill microros-agent
python3 plot_result.py $directory
