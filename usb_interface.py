import os
import sys
import serial
from pynput import keyboard

# NEED TO SEND / RECEIVE on : off

ser = serial.Serial('/dev/cu.usbmodem1421')  # open serial port

def on_press(key):
    try:
        ser.write(bytearray(key.char.upper(), 'utf-8'))
        line = ser.readline()
        print(line)
    except AttributeError:
        print('special key {0} pressed'.format(key))

def on_release(key):
    try:
        ser.write(bytearray(key.char.lower(), 'utf-8'))
        line = ser.readline()
        print(line)
    except:
        pass

    # [ESC to quit]:
    if key == keyboard.Key.esc:
        # Stop listener
        return False

# ...or, in a non-blocking fashion:
listener = keyboard.Listener(on_press=on_press, on_release=on_release)
listener.start()
listener.join()