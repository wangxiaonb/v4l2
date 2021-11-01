import v4l2
import numpy as np
import cv2
import sys
import time
import os
import threading

import RPi.GPIO as GPIO

# os.system('v4l2-ctl --set-fmt-video=width=640,height=480,pixelformat=GREY')

# User Controls
exposure = 0x00980911  # (int) : min=4 max=906 step=1 default=800 value=800
horizontal_flip = 0x00980914  # (bool): default=0 value=0
vertical_flip = 0x00980915  # (bool): default=0 value=0

# (int) : min=0 max=32765 step=1 default=-8193 value=240
brightness = 0x00980900
gain = 0x00980913  # (int): min=16 max=64 step=1 default=57343 value=32

# Camera Controls
# (menu):min=0 max=2 default=2 value=2 flags=read-only
camera_orientation = 0x009a0922
# (int):min=0 max=0 step=1 default=0 value=0 flags=read-only
camera_sensor_rotation = 0x009a0923

# Image Source Controls
# (int) : min=21 max=32367 step=1 default=21 value=21
vertical_blanking = 0x009e0901
# (int) : min=816 max=816 step=1 default=816 value=816 flags=read-only
horizontal_blanking = 0x009e0902
analogue_gain = 0x009e0903  # (int) : min=16 max=248 step=1 default=16 value=16

# Image Processing Controls
# (intmenu): min=0 max=0 default=0 value=0 flags=read-only
link_frequency = 0x009f0901
# (int64)  : min=200000000 max=200000000 step=1 default=200000000 value=2000000
pixel_rate = 0x009f0902


def thread_show():
    global g_image
    global image
    global semaphore
    while True:
        semaphore.acquire()

        # lock.acquire()
        # image_show = g_image
        # lock.release()

        cv2.imshow('image', g_image)

        key = cv2.waitKey(1)
        if key == ord('q'):
            sys.exit()


def main():
    global image
    global g_image
    global semaphore

    t1 = threading.Thread(target=thread_show)

    camera = v4l2.open2('/dev/video2', width, height, 'YUYV')
    v4l2.start(camera)
    # v4l2.setformat(camera,width,height,'GREY')

    # v4l2.setcontrol(camera, exposure, 2900)
    # value = v4l2.getcontrol(camera, exposure)

    v4l2.setcontrol(camera, brightness, 240)
    v4l2.setcontrol(camera, gain, 32)

    # v4l2.setcontrol(camera, V4L2_CID_HBLANK, 800)
    # value = v4l2.getcontrol(camera,V4L2_CID_HBLANK)

    # value = v4l2.getcontrol(camera, vertical_blanking)
    # v4l2.setcontrol(camera, vertical_blanking, 3000)
    # value = v4l2.getcontrol(camera, vertical_blanking)

    t1.start()

    frame_count = 0

    fps_count = 0
    fps = 0
    ts = time.time()

    # while frame_count < 1000:
    while True:
        GPIO.output(17, False)
        GPIO.output(18, False)

        data = v4l2.read(camera)

        GPIO.output(17, True)
        GPIO.output(18, True)

        # time.sleep(0.005)

        fps_count += 1
        if fps_count >= 30:
            t = time.time() - ts
            fps = int(round(fps_count / t, 0))
            print("fps:%d   frame length:%d" % (fps, len(data)))
            fps_count = 0
            ts = time.time()

        frame_count += 1
        if frame_count >= 1:
            frame_count = 0

            image_array = np.frombuffer(data, dtype=np.uint8)
            array0 = image_array[0::2]
            array1 = image_array[1::2]
            image0 = array0.reshape(height, width)
            image1 = array1.reshape(height, width)
            # image = np.hstack((image0, image1))
            image = image0

            # image = cv2.resize(image, (960, 360))
            # image = cv2.flip(image, 0)

            image = cv2.cvtColor(image, cv2.COLOR_GRAY2BGR)
            image = cv2.putText(image, 'FPS:'+str(fps), (10, 30),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (80, 80, 240), 1)

            lock.acquire()
            g_image = image
            lock.release()
            semaphore.release()

        cv2.imshow('image', image)
        key = cv2.waitKey(1)
        if key == ord('q'):
            sys.exit()

    v4l2.stop(camera)
    v4l2.close(camera)


gpio_state = True
GPIO.setmode(GPIO.BCM)
GPIO.setup(17, GPIO.OUT)
GPIO.setup(18, GPIO.OUT)

# width, height = 1280, 800
# width, height = 1280, 720
width, height = 640, 480
g_image = np.zeros((height, width), dtype=np.uint8)
lock = threading.Lock()
semaphore = threading.Semaphore(0)

if __name__ == '__main__':
    main()
