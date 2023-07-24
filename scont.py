import serial
ser = serial.Serial('/dev/ttyS0', 115200)

while True:
	command = input()
	command = command + "\n"
	ser.write(command.encode())
