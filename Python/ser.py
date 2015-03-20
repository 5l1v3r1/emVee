import serial
import select 
import sys
import termios
import time

ser = serial.Serial('/dev/tty.usbserial-A6026OWM', 57600, timeout=2, xonxoff=False, rtscts=False, dsrdtr=False)
f = open ("Output.txt","w")
now = time.time()
f.write ("STARTING TIME: " + now +  "\n")
while True :
    inp, outp, err = select.select([sys.stdin, ser], [], [], .2)

    if sys.stdin in inp :
        line = sys.stdin.read(1)
        ser.write(line)

    if ser in inp :
		line = ser.readline().strip()
		print line
		f.write(line)

now = time.time()
f.write("ENDING TIME: " + now + "\n")
f.close()
