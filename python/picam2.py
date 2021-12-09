
import threading
from imutils.video import VideoStream, videostream
import imutils
import time
import numpy as np
import cv2
 
# # 为线程定义一个函数
def camera_thread(dev):
    cap = VideoStream(dev).start()
    n=0

    fps_count = 0
    fps = 0
    ts = time.time()
    while True:
        img = cap.read()
        n=n+1

        fps_count += 1
        if fps_count >= 200:
            t = time.time() - ts
            fps = int(round(fps_count / t, 0))
            print(dev+" fps:%d" % (fps))
            fps_count = 0
            ts = time.time()

        img = cv2.putText(img, 'FPS:'+str(fps), (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (220, 220, 0), 1)

        cv2.putText(img, str(n), (50, 300), cv2.FONT_HERSHEY_SIMPLEX, 1.2, (255, 255, 255), 2)
        cv2.imshow(dev, img)
        cv2.waitKey(1)

    cap.release()


t0 = threading.Thread(target=camera_thread,args=('/dev/video0',))
t1 = threading.Thread(target=camera_thread,args=('/dev/video1',))

t0.start()
t1.start()

# t0.join()
# t1.join()

time.sleep(0)
