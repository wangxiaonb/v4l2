import v4l2
import numpy as np
import cv2
import sys
import time


width, height = 640, 400

camera = v4l2.open('/dev/video0')
v4l2.start(camera)


fps_count = 0
fps = 0
bps = 0
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

    image = cv2.putText(image, 'FPS:'+str(fps), (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (220, 220, 0), 1)

    cv2.imshow('image', image)

    key = cv2.waitKey(1)
    if key == ord('q'):
        sys.exit()


v4l2.stop(camera)
v4l2.close(camera)
