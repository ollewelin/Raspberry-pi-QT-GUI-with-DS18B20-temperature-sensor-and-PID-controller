#!/usr/bin/env python3

""" heatpump_server.py -
    This code do this
    1. start a socket server for communicate with a socket (from a GUI QT application with a heatpump_socket_clinet.cpp class)
    2. Inside this socket server while True: loop command and data reply will be handle with use of the tinytuya API
    (a tuya API how use the LocalKey not using the Cloud)

    The command and reply data is in a form of 10 array c_uint32 data send/recive
    array index [0] = "command"
        0=do nothig
        1=Turn OFF heatpump device
        2=Turn ON heatpump device
        3=hot_hotwater mode (heater + hotwater mode selected)
        4=hotwater mode (Only hotwater mode selected)
        5=hot mode (Only heater mode)
        6=set_temp
        7=only status read

        10=cool mode, not implemented yet
        11=cool_hotwater mode, not implemented yet
    array index [1] = setpoint temperature
    array index [2] = readback ON/OFF
    array index [3] = readback actual temperature
    array index [4] = readback setpoint temperature
    array index [5] = readback mode, mode readback code same as array index [0] = "command"
"""

import socket
import sys
import random
from ctypes import *

import tinytuya
import time
import copy
device = tinytuya.OutletDevice('<Tuya device ID>', '192.168.xx.xxx', '<LocalKey>')

#device.set_socketPersistent(True)
def parse_tuya_int(ux_id_string, tuya_string):
    number = 0
    li = list(tuya_string.split(ux_id_string))
    if len(li)>1:
        li2 = list(li[1].split(","))
        if li2[0].isdigit():
            number = int(li2[0])
        else:
            print("ERROR parse. Not digits, could not parse string\n")
            number = -1000
            print("number = {}\n".format(number))
    else:
        print("ERROR parse. To few argument, missing comma separator in string, could not parse string\n")
        #print(number)
    return number


""" This class defines a C-like struct """
class Payload(Structure):
    _fields_ = [("command", c_int32),
                ("index1", c_int32),
                ("index2", c_int32),
                ("index3", c_int32),
                ("index4", c_int32),
                ("index5", c_int32),
                ("index6", c_int32),
                ("index7", c_int32),
                ("index8", c_int32),
                ("index9", c_int32)]


def main():
    PORT = 2300
    server_addr = ('localhost', PORT)
    ssock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print("Socket created")
    payload_pre = Payload
    try:
        # bind the server socket and listen
        ssock.bind(server_addr)
        print("Bind done")
        ssock.listen(3)
        print("Server listening on port {:d}".format(PORT))

        while True:
            csock, client_address = ssock.accept()
            print("Accepted connection from {:s}".format(client_address[0]))

            buff = csock.recv(512)
            while buff:
                
                

                device.set_version(3.3)
                data = device.status()
                #device.set_socketPersistent(True)
                
                print("\nReceived {:d} bytes".format(len(buff)))
                payload_in = Payload.from_buffer_copy(buff)
                print("Received contents command={}, i1={}, i2={}, i3={}, i={}, i5={}, i6={}, i7={}, i8={}, i9={}".format(payload_in.command,
                                                            payload_in.index1,
                                                            payload_in.index2,
                                                            payload_in.index3,
                                                            payload_in.index4,
                                                            payload_in.index5,
                                                            payload_in.index6,
                                                            payload_in.index7,
                                                            payload_in.index8,
                                                            payload_in.index9))
           #     if payload_in != payload_pre :
                   # payload_pre = Payload.from_buffer_copy(buff)
                if payload_in.command == 1:
                    device.turn_off(switch=1)
                    print('Turn OFF heatpump')
                elif payload_in.command == 2:
                    device.turn_on(switch=1)
                    print('Turn ON heatpump')
                elif payload_in.command == 3:
                    device.turn_on(switch=1)
                    print('Turn ON heatpump')
                    device.set_value(4,'hot_hotwater')
                    print('hot_hotwater mode')
                elif payload_in.command == 4:
                    device.turn_on(switch=1)
                    print('Turn ON heatpump')
                    device.set_value(4,'hotwater')
                    print('hotwater mode')
                elif payload_in.command == 5:
                    device.turn_on(switch=1)
                    print('Turn ON heatpump')
                    device.set_value(4,'hot')
                    print('hot mode')
                print("debug\n")
                #payload_in.index1 = 55
                device.set_value(2,payload_in.index1)
                print("set temperature to {}".format(payload_in.index1))
                        
                time.sleep(1) # Sleep for 1 seconds
                data = device.status()
                print('set_status() result %r' % data)
                #print("status string = ", data, "\n")
                parse_string = str(copy.deepcopy(data))
                payload_in.index3 = parse_tuya_int("u'3': ", parse_string)
                payload_in.index4 = parse_tuya_int("u'2': ", parse_string)

                print("Send contents back, command={}, i1={}, i2={}, i3={}, i={}, i5={}, i6={}, i7={}, i8={}, i9={}".format(payload_in.command,
                                                            payload_in.index1,
                                                            payload_in.index2,
                                                            payload_in.index3,
                                                            payload_in.index4,
                                                            payload_in.index5,
                                                            payload_in.index6,
                                                            payload_in.index7,
                                                            payload_in.index8,
                                                            payload_in.index9))

                print("Sending it back.. \n")
                nsent = csock.send(payload_in)
                print("Sent {:d} bytes".format(nsent))
                buff = csock.recv(512)

            print("Closing connection to client")
            print("----------------------------")
            csock.close()

    except AttributeError as ae:
        print("Error creating the socket: {}".format(ae))
    except socket.error as se:
        print("Exception on socket: {}".format(se))
    except KeyboardInterrupt:
        ssock.close()
    finally:
        print("Closing socket")
        ssock.close()


if __name__ == "__main__":
    main()
