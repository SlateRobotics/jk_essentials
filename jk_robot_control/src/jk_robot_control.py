#!/usr/bin/env python

import time
import rospy
import sys
import signal
import glob
import serial
import numpy as np
import math
import datetime
import subprocess
from sensor_msgs.msg import Joy
from geometry_msgs.msg import Twist
from std_msgs.msg import String
from std_msgs.msg import Float64

close = False

jk_mode_pub = None

CONTROL_MSG_XBOX = "REST"
CONTROL_MSG_TWITCH = "TURBO"

while len(glob.glob('/dev/ttyACM*')) < 1:
	print("Waiting for Arduino connections. Retrying every 1 sec.")
	time.sleep(1.0)

devA = glob.glob('/dev/ttyACM*')
devA1 = devA[0]

print("Connecting to " + devA1)

flag_debounce = datetime.datetime.now()
flag_xbox_control = None

joy_data = None
twitch_data = None
last_twitch_data = datetime.datetime.now()
last_joy_data = datetime.datetime.now()

arduino = serial.Serial(devA1,
						baudrate = 115200,
						bytesize = 8,
						parity = serial.PARITY_NONE,
						stopbits = serial.STOPBITS_ONE,
						timeout = 1)

def signal_handler(sig, frame):
	global close
	close = True
	sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

def subscriber_callback(data):
	global joy_data, last_joy_data, flag_xbox_control, flag_debounce, jk_mode_pub
	joy_data = data
	last_joy_data = datetime.datetime.now()

	current_time = datetime.datetime.now()
	t_delta = current_time - flag_debounce
	flag_debounce_delta = t_delta.total_seconds()

	prev_mode = flag_xbox_control

	# XBOX-Button: give control to twitch
	if (data.buttons[8] == 1):
		flag_xbox_control = False
		flag_debounce = datetime.datetime.now()
	elif (flag_debounce_delta > 0.250):
		flag_xbox_control = True

	if prev_mode != flag_xbox_control:
		if flag_xbox_control == True:
			jk_mode_pub.publish(CONTROL_MSG_XBOX)
		else:
			jk_mode_pub.publish(CONTROL_MSG_TWITCH)

# input: numpy array, -1 to 1 value
def getMotorValues(y, rotationStrength):
	desiredMagnitude = abs(y)
	if (abs(rotationStrength) > desiredMagnitude):
		desiredMagnitude = abs(rotationStrength)

	if (desiredMagnitude == 0):
		return 0, 0

	# define motors
	left = y
	right = y

	#manipulate values based on rotation from rotationStrength
	rotationStrength = rotationStrength * 2
	left = left + rotationStrength
	right = right - rotationStrength

	# get output magnitude
	outputMagnitude = abs(right)
	if abs(left) > outputMagnitude:
		outputMagnitude = abs(left)

	# scale output to match desired magnitude
	scale = desiredMagnitude / outputMagnitude

	left = left * scale
	right = right * scale

	scaledOutput = (left, right)

	return scaledOutput

def teleop():
	global mode, flag_xbox_control, joy_data, last_joy_data, twitch_data, last_twitch_data

	bl = 0
	br = 0

	if flag_xbox_control == True:
		if (joy_data == None):
			return

		current_time = datetime.datetime.now()
		t_delta = current_time - last_joy_data
		last_joy_data_delta = t_delta.total_seconds()

		cmdX = joy_data.axes[6]
		cmdY = joy_data.axes[7]

		if cmdX == 0:
			cmdX = joy_data.axes[0]

		if cmdY == 0:
			cmdY = joy_data.axes[1]

		if abs(cmdX) > 0.1 or abs(cmdY) > 0.1:
			y = int(math.floor(cmdY * 100.0))
			rotation = math.floor(cmdX * 100.0)
			bl, br = getMotorValues(y, rotation)
			bl = bl * 0.75
			br = br * 0.75
	else:
		if (twitch_data == None):
			return

		current_time = datetime.datetime.now()
		t_delta = current_time - last_twitch_data
		last_twitch_data_delta = t_delta.total_seconds()

		if last_twitch_data_delta < 1.0/1.5:
			y = int(math.floor(twitch_data.linear.y * 100.0))
			rotation = math.floor(twitch_data.linear.x * -100.0)
			bl, br = getMotorValues(y, rotation)

	bl = int(bl)
	br = int(br)

	if (abs(bl) + abs(br)) < 0.1:
		return

	msgString = str(bl) + ',' + str(br) + ',;'

	if (arduino.isOpen() == False):
			arduino.open()

	for c in msgString:
		arduino.write(c)
		time.sleep(0.001)

def twitch_cmd_cb(data):
	global twitch_data, last_twitch_data
	last_twitch_data = datetime.datetime.now()
	twitch_data = data

def program():
	global close, flag_xbox_control, jk_mode_pub
	rospy.init_node('ip_xbox_teleop', anonymous=True)
	jk_mode_pub = rospy.Publisher("/jk/mode", String, queue_size=10)
	rospy.Subscriber("joy", Joy, subscriber_callback)
	rospy.Subscriber("/jk/twitch_cmd", Twist, twitch_cmd_cb)

	flag_xbox_control = True

	while close != True:
		teleop()
		time.sleep(0.150)

if __name__ == '__main__':
	while True:
		try:
			program()
		except Exception as e:
			print e
			print("Program exception occurred. Restarting.")
			pass
		else:
			print("Program exited. Restarting.")
			break
		time.sleep(3.0)
