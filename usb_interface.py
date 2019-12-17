import os
import sys
import time
import serial
import contextlib
import omni_listener

#https://docs.particle.io/support/particle-devices-faq/photon-serial2-faq/#using-the-serial-port

# HATS for Joystick.c (Left buttons, BOOYA, etc)
'''
# https://github.com/LightningStalker/Splatmeme-Printer/blob/master/Joystick.c
if (ReportData->HAT == HAT_RIGHT)
	xpos++;
else if (ReportData->HAT == HAT_LEFT)
	xpos--;
else if (ReportData->HAT == HAT_TOP)
	ypos--;
else if (ReportData->HAT == HAT_BOTTOM)
	ypos++;
'''

# [Neat helper function for timing operations!]:
@contextlib.contextmanager
def timer(msg):
    start = time.time()
    yield
    end = time.time()
    print('%s: %.02fms'%(msg, (end-start)*1000))

if _SERIAL:
    ser = serial.Serial('/dev/cu.usbmodem1421')  # open serial port

# Add Mouse controller code!
# Add ENUM for PIN names 
# ^(Add configuration for keys, etc)
# OMNI listener.. need to define _state buttons, _timer and _elapsed variables for each key, to determine how long it was pressed, for record/replay
# ^(Build OMNI_REPORT with this data, basically what all keys were pressed at that second)
# ^(Maybe OMNI_REPORT is a DICT with the KEY being the timestamp and each of the keys(or inputs) defined as 1/0 at that second?)

# [0]: Talk to Atmel over VIN/serial with PIN (?)
# [1]: Record/Replay @ usb_interface.py level /w TIMERS??
# [2]: (SPARK-level usb_interface): Rename "w,W; a,A; s,S etc to PIN names like d3_d1,D3_D1, etc)
# [2.5]: Key Config for usb interface (PYTHON-level) to output to certain PINs. <(Taking the config from the photon to python-level for reprogrammable keys) 
if __name__ == "__main__":
    _OMNI = omni_listener.omni_listener()

    # [Start non-blocking listeners]:
    with _OMNI.mouse_listener, _OMNI.keyboard_listener:
        print('[Mouse/Keyboard listening! Press ESC to stop]')
        while True:
            time.sleep(1) #pass (?)

    # [Start non-blocking listener]:
    #listener = keyboard.Listener(on_press=on_press, on_release=on_release)
    #listener.start()
    #listener.join()