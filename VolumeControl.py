import serial
import os

def VolumeChange(dir,inc):
	if dir:	dir_="+"
	else: dir_="-"
	os.system(f"pactl set-sink-volume 0 {dir_}{inc}%")

ser = serial.Serial('/dev/ttyS0', 115200)

while True:	
	data=ser.readline()
	VolumeChange(int(data.decode()[0]),1)
