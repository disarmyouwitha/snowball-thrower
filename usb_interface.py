import os
import sys
import time
import serial
import contextlib
import omni_listener

# [Neat helper function for timing operations!]:
@contextlib.contextmanager
def timer(msg):
    start = time.time()
    yield
    end = time.time()
    print('%s: %.02fms'%(msg, (end-start)*1000))

# [1]: Talk to Atmel over VIN/serial with PIN"
#      # https://docs.particle.io/support/particle-devices-faq/photon-serial2-faq/#using-the-serial-port
#      # https://www.pjrc.com/teensy/td_uart.html
#      # https://community.particle.io/t/serial-tutorial/26946
#      # https://forum.pjrc.com/threads/32502-Serial2-Alternate-pins-26-and-31
#      # pjrc.com/teensy/uart.html
# [2]: Record/Replay @ usb_interface.py level /w TIMERS??
#      OMNI listener.. need to define _state buttons, _timer and _elapsed variables for each key, to determine how long it was pressed, for record/replay
#      ^(Build OMNI_REPORT with this data, basically what all keys were pressed at that second)
#      ^(Maybe OMNI_REPORT is a DICT with the KEY being the timestamp and each of the keys(or inputs) defined as 1/0 at that second?)
if __name__ == "__main__":
    _OMNI = omni_listener.omni_listener()

    # [Start non-blocking listeners]:
    with _OMNI.mouse_listener, _OMNI.keyboard_listener:
        print('[Mouse/Keyboard listening! Press ESC to stop]')
        while True:
            time.sleep(1) #pass (?)