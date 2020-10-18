import socket
import sys

HOST = 'localhost'
PORT = 2210

# Create a Datagram (UDP) socket
try :
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print 'Socket created'
except socket.error, msg :
    print 'Failed to create socket. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
    sys.exit()

# Bind socket to HOST and PORT
try:
    s.bind((HOST, PORT))
except socket.error , msg:
    print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
    sys.exit()

while 1:

    (string, address) = s.recvfrom(9)
    frame = bytes(string)
    header1 = ord(frame[0])
    header2 = ord(frame[1])

    distance_low = ord(frame[2])
    distance_high = ord(frame[3])
    distance_high <<= 8
    distance = distance_low + distance_high

    strength_low = ord(frame[4])
    strength_high = ord(frame[5])
    strength_high <<= 8
    strength = strength_low + strength_high

    checksum = ord(frame[8])

    if header1 + header2 != (0x59 + 0x59):
        print("error in headers")

    print str(distance) + ' ' + str(strength)
t.close()
