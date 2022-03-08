import v4l2
import numpy as np
import cv2
import sys
import time
import argparse

# os.system('v4l2-ctl --set-fmt-video=width=640,height=480,pixelformat=GREY')

# User Controls
exposure = 0x00980911  # (int) : min=4 max=906 step=1 default=800 value=800
horizontal_flip= 0x00980914 #(bool): default=0 value=0
vertical_flip= 0x00980915 #(bool): default=0 value=0

# Camera Controls
camera_orientation =0x009a0922 #(menu):min=0 max=2 default=2 value=2 flags=read-only
camera_sensor_rotation =0x009a0923 #(int):min=0 max=0 step=1 default=0 value=0 flags=read-only

# Image Source Controls
vertical_blanking =0x009e0901 #(int) : min=21 max=32367 step=1 default=21 value=21
horizontal_blanking =0x009e0902 #(int) : min=816 max=816 step=1 default=816 value=816 flags=read-only
analogue_gain =0x009e0903 #(int) : min=16 max=248 step=1 default=16 value=16

# Image Processing Controls
link_frequency= 0x009f0901 #(intmenu): min=0 max=0 default=0 value=0 flags=read-only
pixel_rate= 0x009f0902 #(int64)  : min=200000000 max=200000000 step=1 default=200000000 value=2000000

##########################################################################
width, height = 1280, 800
# width, height = 1280, 720
# width, height = 640, 480
# width, height = 640, 400

##############################################################################
parser = argparse.ArgumentParser(description='camera display')
parser.add_argument('-n', '--number', type=str,
                    default='0', help='camera number')
args = parser.parse_args()

camera_num = args.number

# camera = v4l2.open('/dev/video0')
camera = v4l2.open2('/dev/video'+camera_num, width, height, 'GREY')
v4l2.start(camera)
# v4l2.setformat(camera,width,height,'GREY')

v4l2.setcontrol(camera, exposure, 2900)
value = v4l2.getcontrol(camera, exposure)

# v4l2.setcontrol(camera, V4L2_CID_HBLANK, 800)
# value = v4l2.getcontrol(camera,V4L2_CID_HBLANK)

# value = v4l2.getcontrol(camera, vertical_blanking)
v4l2.setcontrol(camera, vertical_blanking, 500)
# value = v4l2.getcontrol(camera, vertical_blanking)

fps_count = 0
fps = 0
ts = time.time()

while True:
    data = v4l2.read(camera)

    fps_count += 1
    if fps_count >= 120:
        t = time.time() - ts
        fps = int(round(fps_count / t, 0))
        print("fps:%d   frame length:%d" % (fps, len(data)))
        fps_count = 0
        ts = time.time()


    image_array = np.frombuffer(data, dtype=np.uint8)
    image = image_array.reshape(height, width)

    # image = cv2.resize(image, (768, 480))
    # image = cv2.flip(image, 0)

    image = cv2.putText(image, 'FPS:'+str(fps), (10, 30),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (127, 127, 127), 1)

    cv2.imshow('camera'+camera_num, image)
    key = cv2.waitKey(1)
    if key == ord('q'):
        break

v4l2.stop(camera)
v4l2.close(camera)
