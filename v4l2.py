import v4l2
import numpy as np
import cv2
import sys
import time
import os
import threading

# os.system('v4l2-ctl --set-fmt-video=width=640,height=480,pixelformat=GREY')

V4L2_CTRL_CLASS_USER = 0x00980000
V4L2_CID_BASE = (V4L2_CTRL_CLASS_USER | 0x900)
V4L2_CID_USER_BASE = V4L2_CID_BASE
V4L2_CID_USER_CLASS = (V4L2_CTRL_CLASS_USER | 1)
V4L2_CID_BRIGHTNESS = (V4L2_CID_BASE+0)
V4L2_CID_CONTRAST = (V4L2_CID_BASE+1)
V4L2_CID_SATURATION = (V4L2_CID_BASE+2)
V4L2_CID_HUE = (V4L2_CID_BASE+3)
V4L2_CID_AUTO_WHITE_BALANCE = (V4L2_CID_BASE+12)
V4L2_CID_DO_WHITE_BALANCE = (V4L2_CID_BASE+13)
V4L2_CID_RED_BALANCE = (V4L2_CID_BASE+14)
V4L2_CID_BLUE_BALANCE = (V4L2_CID_BASE+15)
V4L2_CID_GAMMA = (V4L2_CID_BASE+16)
V4L2_CID_EXPOSURE = (V4L2_CID_BASE+17)
V4L2_CID_AUTOGAIN = (V4L2_CID_BASE+18)
V4L2_CID_GAIN = (V4L2_CID_BASE+19)
V4L2_CID_HFLIP = (V4L2_CID_BASE+20)
V4L2_CID_VFLIP = (V4L2_CID_BASE+21)

V4L2_CTRL_CLASS_IMAGE_SOURCE = 0x009e0000
V4L2_CID_IMAGE_SOURCE_CLASS_BASE = (V4L2_CTRL_CLASS_IMAGE_SOURCE | 0x900)
V4L2_CID_VBLANK = (V4L2_CID_IMAGE_SOURCE_CLASS_BASE + 1)
V4L2_CID_HBLANK = (V4L2_CID_IMAGE_SOURCE_CLASS_BASE + 2)


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

    # camera = v4l2.open('/dev/video0')
    camera = v4l2.open2('/dev/video0', width, height, 'GREY')
    v4l2.start(camera)
    # v4l2.setformat(camera,width,height,'GREY')

    v4l2.setcontrol(camera, V4L2_CID_EXPOSURE, 800)
    value = v4l2.getcontrol(camera,V4L2_CID_EXPOSURE)

    # v4l2.setcontrol(camera, V4L2_CID_HBLANK, 800)
    # value = v4l2.getcontrol(camera,V4L2_CID_HBLANK)

    value = v4l2.getcontrol(camera,V4L2_CID_VBLANK)
    v4l2.setcontrol(camera, V4L2_CID_VBLANK, 1500)
    value = v4l2.getcontrol(camera,V4L2_CID_VBLANK)


    t1.start()

    frame_count = 0

    fps_count = 0
    fps = 0
    ts = time.time()
    while True:
        data = v4l2.read(camera)

        fps_count += 1
        if fps_count >= 120:
            t = time.time() - ts
            fps = int(round(fps_count / t, 0))
            print("fps:", fps)
            fps_count = 0
            ts = time.time()

        frame_count += 1
        if frame_count >= 0:
            frame_count = 0

            image_array = np.frombuffer(data, dtype=np.uint8)
            image = image_array.reshape(height, width)
            image = cv2.flip(image, 0)

            image = cv2.putText(image, 'FPS:'+str(fps), (10, 30),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (220, 220, 0), 1)
            lock.acquire()
            g_image = image
            lock.release()
            semaphore.release()

            # cv2.imshow('image', image)
            # key = cv2.waitKey(1)
            # if key == ord('q'):
            #     sys.exit()

    v4l2.stop(camera)
    v4l2.close(camera)


width, height = 640, 400
g_image = np.zeros((height, width), dtype=np.uint8)
lock = threading.Lock()
semaphore = threading.Semaphore(0)

if __name__ == '__main__':
    main()
