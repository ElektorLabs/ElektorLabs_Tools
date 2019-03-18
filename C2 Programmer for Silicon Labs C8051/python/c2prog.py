# Script to send a HEX file to an Arduino running the c2prog.ino sketch.
#
# usage: c2prog.py [-h] [-v] port file
# 
# positional arguments:
#   port        serial port name
#   file        input file in Intel HEX format
# 
# optional arguments:
#   -h, --help  show this help message and exit
#   -v          verbose
# 
# Written by CPV 
# (C) Elektor, 2019

import argparse
import serial
import time

baudrate = 115200
c2_reset = ':00000001FF'

def main():
	parser = argparse.ArgumentParser(
		description='HEX file uploader for Elektor C2 programmer.',
		epilog='(C) Elektor, 2019.'
		)
	parser.add_argument('port', help='serial port name')
	parser.add_argument('file', help='input file in Intel HEX format')
	parser.add_argument('-v', dest='verbose', action='store_true', help='verbose')
	args = parser.parse_args()

	try:
		# Open serial port with read timeout of 5 seconds.
		s = serial.Serial(args.port,baudrate,timeout=5)
		s.dtr = 0 # Reset Arduino.
		s.dtr = 1
		print("Connecting...");
		time.sleep(2) # Wait for Arduino to finish booting.
		s.reset_input_buffer() # Delete any stale data.
	except:
		print("Could not open serial port:",args.port)
		return None

	try:
		with open(args.file) as f:
			for lines in f:
				line = lines.rstrip() # Remove trailing whitespace.
				if (args.verbose==True):
					print(line)
				s.write(line.encode());
				ack = s.read();
				if (ack==b''):
					print("Operation timed out, aborting")
					break;
				if (args.verbose==True):
					print(ack.decode())
			s.write(c2_reset.encode()); # Try to reset C2 target.
			f.close()
	except FileNotFoundError:
		print("Invalid filename or file path:",args.file)
		return None
	
if __name__ == '__main__':
	main()
print("Done.")
