
import serial
import time
ser= serial.Serial("/dev/ttyUSB1", 115200)
ser.write(">OUT:1,1\r\n")

time.sleep(5)
ser.close





