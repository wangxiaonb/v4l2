# import the necessary packages
from picamera.array import PiRGBArray
from picamera import PiCamera
import time
import cv2
import numpy as np
import sys

# initialize the camera and grab a reference to the raw camera capture
camera = PiCamera()

camera.flash_mode = 'on'
camera.capture('foo.jpg')
sys.exit()

camera.resolution = (640, 400)
camera.framerate = 60
camera.hflip = True
camera.vflip = True
rawCapture = PiRGBArray(camera, size=(640, 400))
# allow the camera to warmup
time.sleep(0.1)
# capture frames from the camera


# import time
# import io
# from time import sleep
# import picamera
# import numpy as np
# import cv2

# with picamera.PiCamera() as camera:
#     camera.resolution = (640, 480)
#     sleep(1)

#     count = 0
#     fps = 0
#     ts = time.time()
#     stream = io.BytesIO()
#     for foo in camera.capture_continuous(stream, format='jpeg', use_video_port=True):
#         data = np.fromstring(stream.getvalue(), dtype=np.uint8)
#         image = cv2.imdecode(data, 3)
        
#         if count >= 60:
#             t = time.time() - ts
#             fps = round(count / t, 0)
#             count = 0
#             ts = time.time()
#         count += 1
        
#         image = cv2.putText(image, 'FPS:'+str(fps), (10, 30),
#                     cv2.FONT_HERSHEY_SIMPLEX, 0.5, (20, 20, 250), 1)

#         cv2.imshow("img", image)
#         cv2.waitKey(1)

#         # Truncate the stream to the current position (in case
#         # prior iterations output a longer image)
#         stream.truncate()
#         stream.seek(0)


count = 0
fps = 0
ts = time.time()
for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
    
    if count >= 60:
        t = time.time() - ts
        fps = round(count / t, 0)
        count = 0
        ts = time.time()
    count += 1

    # grab the raw NumPy array representing the image, then initialize the timestamp
    # and occupied/unoccupied text
    image = frame.array

    image = cv2.putText(image, 'FPS:'+str(fps), (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (20, 20, 250), 1)

    # show the frame
    cv2.imshow("Frame", image)
    key = cv2.waitKey(1) & 0xFF
    # clear the stream in preparation for the next frame
    rawCapture.truncate(0)
    # if the `q` key was pressed, break from the loop
    if key == ord("q"):
        break
