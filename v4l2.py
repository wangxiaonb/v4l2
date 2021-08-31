import v4l2
import numpy as np
import cv2
import sys
import time
import os
import threading

# os.system('v4l2-ctl --set-fmt-video=width=640,height=480,pixelformat=GREY')


def thread_show():
    global g_image
    while True:
        lock.acquire()
        image_show = g_image
        lock.release()

        cv2.imshow('image', image_show)

        cv2.waitKeyEx(1)

        # key = cv2.waitKey(1)
        # if key == ord('q'):
        #     sys.exit()


def main():
    global g_image

    t1 = threading.Thread(target=thread_show)

    # camera = v4l2.open('/dev/video0')
    camera = v4l2.open2('/dev/video0', width, height, 'GREY')
    v4l2.start(camera)
    # v4l2.setformat(camera,width,height,'GREY')

    t1.start()

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

        image_array = np.frombuffer(data, dtype=np.uint8)
        image = image_array.reshape(height, width)
        image = cv2.flip(image,0)

        image = cv2.putText(image, 'FPS:'+str(fps), (10, 30),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (220, 220, 0), 1)

        lock.acquire()
        g_image = image
        lock.release()

        # cv2.imshow('image', image)

        # key = cv2.waitKey(1)
        # if key == ord('q'):
        #     sys.exit()

    v4l2.stop(camera)
    v4l2.close(camera)


width, height = 1280, 800
g_image = np.zeros((height, width), dtype=np.uint8)
lock = threading.Lock()

if __name__ == '__main__':
    main()
