import urllib.request
import _thread
import cv2
import numpy as np
import time

from controller import calculate_control_signal, send_control, set_control_ip
from config import CAM_URL

stream = urllib.request.urlopen(CAM_URL)
bytes = bytes()

# Set control ip continuously to receive sensor params
def set_control_ip():
    time.sleep(2)
    set_control_ip()
_thread.start_new_thread(set_control_ip, ())

while True:

    bytes += stream.read(1024)
    a = bytes.find(b'\xff\xd8')
    b = bytes.find(b'\xff\xd9')

    if a != -1 and b != -1:
        image = bytes[a:b+2]
        bytes = bytes[b+2:]

        try:
            # Decode image
            image = cv2.imdecode(np.fromstring(
                image, dtype=np.uint8), cv2.IMREAD_COLOR)
            cv2.imshow('Image', image)
        except:
            if cv2.waitKey(1) == 27:
                exit(0)
            continue

        # Calculate speed and steering angle
        steering = calculate_control_signal(image.copy())

        if steering is None:
            steering = 0

        send_control(steering)

        if cv2.waitKey(1) == 27:
            exit(0)
