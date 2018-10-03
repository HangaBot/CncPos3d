#!/usr/bin/python
# Author: Alfonso Breglia
# Release Date: 01/10/18
# Description: Software di controllo per stampante 3d
# TODO: aggiungere algoritmo per tracciare una linea
import serial
import time
import sys


class CncPos3d:
    def __init__(self, port, debug = False):
        self.debug = debug
        self.ser = serial.Serial(port)
        time.sleep(2)
        res = self.ser.readline()
        print(res)
    
    def moveX(self, pos):
        cmd_pos = int(pos) & 0xFFFF
        cmd = "!X{:04X}\n".format(cmd_pos).encode()
        if self.debug:
            print(cmd)
        self.ser.flushInput()
        self.ser.write(cmd)
        res = self.ser.readline()
        if self.debug:
            print(res)
        if res != b":OK\n":
            print("Error on MoveX")


    
    def moveY(self, pos):
        cmd_pos = int(pos) & 0xFFFF
        cmd = "!Y{:04X}\n".format(cmd_pos).encode()
        self.ser.flushInput()
        if self.debug:
            print(cmd)
        self.ser.write(cmd)
        res = self.ser.readline()
        if self.debug:
            print(res)
        if res != b":OK\n":
            print("Error on MoveY")

        
    def moveZ(self, pos):
        cmd_pos = int(pos) & 0xFFFF
        cmd = "!Z{:04X}\n".format(cmd_pos).encode()
        self.ser.flushInput()
        if self.debug:
            print(cmd)
        self.ser.write(cmd)
        res = self.ser.readline()
        if self.debug:
            print(res)
        if res != b":OK\n":
            print("Error on MoveZ")
        
        
        
if __name__ == "__main__":

    print (sys.argv)
    cnc = CncPos3d("COM6",True)
    
    if(len (sys.argv) == 2 ):
    
        filename = sys.argv[1]
        sequence_file = open(filename, "r")
        cmds = sequence_file.readlines()
        for l in cmds:
            time.sleep(1)
            l = l.strip().split()
            if len(l) == 0 or l[0] == "REM":
                continue
            elif l[0] == "X":
                cnc.moveX(int(l[1]))
            elif l[0] == "Y":
                cnc.moveY(int(l[1]))

            elif l[0] == "PEN":
                if(l[1] == "UP"):
                    cnc.moveZ(6000)
                elif (l[1] == "DOWN"):
                    cnc.moveZ(-3000)
                else:
                    print("SYNTAX ERROR")
            else:
                print("SYNTAX ERROR")
            
    else:
        print("Usage: python CncPos3dCtrl.py xxx.seq")
    
