#Get button data from Chronos watch.
#Taken from info posted at: http://e2e.ti.com/support/microcontrollers/msp43016-bit_ultra-low_power_mcus/f/166/t/32714.aspx
#
#posted here: http://pastebin.com/m5d1b7ced
#Written by Sean Brewer (seabre)

#modified by Oliver Smith 
#http://www.chemicaloliver.net

#seabre986@gmail.com

#

import serial

import array

def startAccessPoint():

    return array.array('B', [0xFF, 0x07, 0x03]).tostring()

def accDataRequest():

    return array.array('B', [0xFF, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00]).tostring()


#Open COM port 6 (check your system info to see which port

#yours is actually on.)

#argments are 5 (COM6), 115200 (bit rate), and timeout is set so

#the serial read function won't loop forever.

# ser = serial.Serial(5,115200,timeout=1)

ser = serial.Serial("/dev/ttyACM0",115200,timeout=1)



#Start access point

ser.write(startAccessPoint())


while True:

    #Send request for acceleration data

    ser.write(accDataRequest())

    accel = ser.read(7)

    unknown = 1 # indicate that the received data is of unkown type



    if len(accel) != 7:

        continue

    if ord(accel[6]) == 1 and ord(accel[5]) == 7 and ord(accel[4]) == 6 and ord(accel[3]) == 255 and ord(accel[2]) == 0 and ord(accel[1]) == 0 and ord(accel[0]) == 0:

	# bogus data?

        unknown = 0

        continue

    if ord(accel[6]) == 18 and ord(accel[5]) == 7 and ord(accel[4]) == 6 and ord(accel[3]) == 255 and ord(accel[2]) == 0 and ord(accel[1]) == 0 and ord(accel[0]) == 0:

	print "M1 pressed"

	M1 = 1

        unknown = 0

    if ord(accel[6]) == 34 and ord(accel[5]) == 7 and ord(accel[4]) == 6 and ord(accel[3]) == 255 and ord(accel[2]) == 0 and ord(accel[1]) == 0 and ord(accel[0]) == 0:

	print "M2 pressed"

	M2 = 1

        unknown = 0

    if ord(accel[6]) == 50 and ord(accel[5]) == 7 and ord(accel[4]) == 6 and ord(accel[3]) == 255 and ord(accel[2]) == 0 and ord(accel[1]) == 0 and ord(accel[0]) == 0:

	print "S1 pressed"

	S1 = 1

        unknown = 0

    if ord(accel[6]) == 255 and ord(accel[5]) == 7 and ord(accel[4]) == 6 and ord(accel[3]) == 255 and ord(accel[2]) == 0 and ord(accel[1]) == 0 and ord(accel[0]) == 0:

        # accelerometer data, but it is bogus

        unknown = 0

        continue

    if ord(accel[6]) == 255 and ord(accel[5]) == 7 and ord(accel[4]) == 6 and ord(accel[3]) == 255:

        print "Accelerometer data: x: " + str(ord(accel[2])) + "\t y: " + str(ord(accel[1])) + "\t z: " + str(ord(accel[0]))

        unknown = 0


        print "Unknown data: 6: " + str(ord(accel[6])) + "\t5: " + str(ord(accel[5])) + "\t4: " + str(ord(accel[4])) + "\t3: " + str(ord(accel[3])) + "\t2: " + str(ord(accel[2])) + "\t1: " + str(ord(accel[1])) + "\t0: " + str(ord(accel[0]))

        continue


ser.close()
