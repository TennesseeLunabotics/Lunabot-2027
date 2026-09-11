import asyncio
import json
import threading

import cv2
import websockets

import rclpy
from rclpy.node import Node
from rclpy.executors import MultiThreadedExecutor

from sensor_msgs.msg import Joy
from sensor_msgs.msg import Image, CompressedImage
from std_msgs.msg import Float32

from cv_bridge import CvBridge


clients = set()
node = None


class WebBridge(Node):

    def __init__(self):
        super().__init__('web_bridge')

        self.joy_pub = self.create_publisher(
            Joy,
            '/joy',
            10
        )

        self.bridge = CvBridge()

        self.latest_frame = None
        self.speed = 0.0

        self.create_subscription(
            CompressedImage,
            '/usb_cam_0/image_raw/compressed',
            self.image_callback,
            1
        )

        self.create_subscription(
            Float32,
            '/shovel/arm_angle',
            self.speed_callback,
            10
        )

    # ---------------------------------------------------
    # JOYSTICK PUBLISH
    # ---------------------------------------------------

    def publish_joy(self, axes, buttons):

        msg = Joy()

        msg.axes = [float(a) for a in axes]
        msg.buttons = [int(b > 0.5) for b in buttons]

        self.joy_pub.publish(msg)

    # ---------------------------------------------------
    # CAMERA CALLBACK
    # ---------------------------------------------------
    
    def image_callback(self, msg):
        try:
            self.latest_frame = bytes(msg.data)
        except Exception as e:
            self.get_logger().error(
            f'Camera conversion error: {e}'
            )

        """
        try:
            yuyv = np.frombuffer(
				msg.data,
				dtype=np.uint8
            ).reshape((msg.height, msg.width, 2))


            frame = cv2.cvtColor(
                yuyv, cv2.COLOR_YUV2BGR_YUYV
            )


            frame = cv2.resize(
            frame,
	            (320, 240)
            )


            success, jpeg = cv2.imencode(
	        '.jpg',
	        frame,
	        [
		    int(cv2.IMWRITE_JPEG_QUALITY),
	        40
        ]
        )

            if success:
                self.latest_frame = jpeg.tobytes()

        except Exception as e:
            self.get_logger().error(
            f'Camera conversion error: {e}'
            )
        """
    # ---------------------------------------------------
    # TELEMETRY CALLBACK
    # ---------------------------------------------------

    def speed_callback(self, msg):

        self.speed = round(msg.data, 2)


# -------------------------------------------------------
# WEBSOCKET HANDLER
# -------------------------------------------------------

async def handler(websocket):

    global clients
    global node

    clients.add(websocket)

    print("Client connected")

    try:

        async for message in websocket:

            # joystick messages are JSON text
            if isinstance(message, str):

                data = json.loads(message)

                axes = data.get("axes", [])
                buttons = data.get("buttons", [])

                node.publish_joy(axes, buttons)

    except Exception as e:

        print(f"Client error: {e}")

    finally:

        clients.remove(websocket)

        print("Client disconnected")


# -------------------------------------------------------
# CAMERA STREAM LOOP
# -------------------------------------------------------

async def camera_loop():

    global clients
    global node

    while True:

        if node.latest_frame and clients:

            dead = set()

            for ws in clients:

                try:
                    # binary websocket frame
                    await ws.send(node.latest_frame)

                except:
                    dead.add(ws)

            clients.difference_update(dead)

        # 10 FPS
        await asyncio.sleep(0.1)


# -------------------------------------------------------
# TELEMETRY LOOP
# -------------------------------------------------------

async def telemetry_loop():

    global clients
    global node

    while True:

        if clients:

            telemetry = {
                "type": "telemetry",
                "speed": node.speed
            }

            msg = json.dumps(telemetry)

            dead = set()

            for ws in clients:

                try:
                    await ws.send(msg)

                except:
                    dead.add(ws)

            clients.difference_update(dead)

        # 5 Hz
        await asyncio.sleep(0.2)


# -------------------------------------------------------
# MAIN
# -------------------------------------------------------

async def main():

    global node

    rclpy.init()

    node = WebBridge()

    executor = MultiThreadedExecutor()

    executor.add_node(node)

    threading.Thread(
        target=executor.spin,
        daemon=True
    ).start()

    print("WebSocket server running on 0.0.0.0:8765")

    asyncio.create_task(camera_loop())

    asyncio.create_task(telemetry_loop())

    async with websockets.serve(
        handler,
        "0.0.0.0",
        8765,
        max_size=None
    ):

        await asyncio.Future()


if __name__ == "__main__":

    asyncio.run(main())
