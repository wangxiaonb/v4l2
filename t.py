import v4l2
import numpy as np
import cv2
import sys
import time


width, height = 640, 400

# camera = v4l2.open('/dev/video0')
camera = v4l2.open2('/dev/video0',width,height,'GREY')

# v4l2.start(camera)

# v4l2.setformat(camera,width,height,'GREY')


