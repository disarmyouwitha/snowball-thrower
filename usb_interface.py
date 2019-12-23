import os
import sys
import time
import omni_listener

# Q (sub) strafe left
# E (sub) strafe right
# L (30hz squidbagging!)
#[Record/Replay @ usb_interface.py level /w TIMERS]:
# OMNI listener.. need to define _state buttons, _timer and _elapsed variables for each key, to determine how long it was pressed, for record/replay
# ^(Build OMNI_REPORT with this data, basically what all keys were pressed at that second)
# ^(Maybe OMNI_REPORT is a DICT with the KEY being the timestamp and each of the keys(or inputs) defined as 1/0 at that second?)
if __name__ == "__main__":
    # [Initialize Listeners]:
    _OMNI = omni_listener.omni_listener()
    _delay= .1

    _OMNI._ms_ctrl.position = (_OMNI._screen_center)

    # [Start non-blocking listeners]:
    with _OMNI.mouse_listener, _OMNI.keyboard_listener:
        print('[Mouse/Keyboard listening! mouse_panic_exit is top-left corner]')
        while True:
            _OMNI.program_clock(.05) # Use main loop from omni_listener

            # [Read available serial input]:
            if _OMNI._SERIAL:
                while _OMNI._SERIAL.in_waiting>0:
                    line = _OMNI._SERIAL.readline()
                    line = line.decode("utf-8")
                    line = line.strip()
                    print(line)

            # [Delay for main loop]:
            time.sleep(_delay)