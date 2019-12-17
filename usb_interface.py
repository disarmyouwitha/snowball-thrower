import os
import sys
import time
import serial
import contextlib
import omni_listener

# [SERIAL]:
# https://docs.particle.io/support/particle-devices-faq/photon-serial2-faq/#using-the-serial-port
# https://www.pjrc.com/teensy/td_uart.html
# https://community.particle.io/t/serial-tutorial/26946
# https://forum.pjrc.com/threads/32502-Serial2-Alternate-pins-26-and-31

# Can write config / translation here.. WASD is actual hard-coded values we should send..

'''
int A0_F6 = A0; // W
int A1_F4 = A1; // A
int A2_F1 = A2; // S
int A3_E6 = A3; // D
int A4_B0 = A4; // J
int A5_B2 = A5; // F
int D0_C7 = D0; // R
int D1_C5 = D1; // SPACE
int D2_C3 = D2; // I (Look Up)
int D3_C1 = D3; // K (Look Down)
int D4_E1 = D4; // Q (sub) strafe left
int D5_D7 = D5; // E (sub) strafe right
int D6_D5 = D6; // U (sub)
int D7_D3 = D7; // M (special)
'''

# [Neat helper function for timing operations!]:
@contextlib.contextmanager
def timer(msg):
    start = time.time()
    yield
    end = time.time()
    print('%s: %.02fms'%(msg, (end-start)*1000))

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
    #listener.join()w