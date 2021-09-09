import time
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)
GPIO.setup(17, GPIO.OUT)
GPIO.output(17, GPIO.HIGH)
GPIO.output(17, GPIO.LOW)


freq = 100
dc = 50
p = GPIO.PWM(17, freq)
p.start(dc)

while True:
    p.ChangeFrequency(freq)
    p.ChangeDutyCycle(dc)

time.sleep(0)

p.stop()
