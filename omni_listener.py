import os
import sys
import time
import serial
import contextlib
import pynput.mouse as ms
import pynput.keyboard as kb
from pynput.mouse import Button, Controller
from pynput.keyboard import Key, Controller

# [Neat helper function for timing operations!]:
@contextlib.contextmanager
def timer(msg):
    start = time.time()
    yield
    end = time.time()
    print('%s: %.02fms'%(msg, (end-start)*1000))

# ^(On-click or key-press start the timer for that specific key)
# ^(On-release end the timer for that specific key / record key&duration)
# with timer('boo'):
#   print('booya!')

class omni_listener():
    # [MOUSE GLOBALS]:
    _last_moved = 0
    _last_click = 0
    _last_int_x = None
    _last_int_y = None

    # [KEYBOARD GLOBALS]:
    _HELD_KEYS = []
    _KEY_MAP = None
    _last_typed = 0
    _last_key = None

    _SERIAL = None

    def __init__(self):
        self.mouse_listener = ms.Listener(on_click=self.on_click, on_move=self.on_move, on_scroll=self.on_scroll)
        self.keyboard_listener = kb.Listener(on_press=self.on_press, on_release=self.on_release)
        # ^(non-blocking mouse/keyboard listener)

        # Change first column to remap key:
        self._KEY_MAP = {
            'W': 'W',     # W| up
            'A': 'A',     # A| down
            'S': 'S',     # S| left
            'D': 'D',     # D| right
            'J': 'J',     # J| Fire!
            'F': 'F',     # F| Squid/Walk toggle
            'R': 'R',     # R| Reset Camera // // (A, Confirm)
            ' ': ' ',     #  | JUMP (B)
            'I': 'I',     # I| Look Up   // (X, Map) 
            'K': 'K',     # K| Look Down // (A, Confirm)
            'Q': 'Q',     # Q| Look Left
            'E': 'E',     # E| Look Right
            'U': 'U',     # U| sub // Right-click
            'M': 'M'      # M| special
        }

        self._SERIAL = serial.Serial('/dev/cu.usbmodem1421')  # open serial port

    # [Mouse position at 0,0 is emergency exit condition]:
    def CHECK_MOUSE_EMERGENCY(self, _int_x, _int_y):
        if _int_x == 0 and _int_y == 0:
            print('[MOUSE_PANIC_EXIT]')

            # Send LOW for all pins when exiting:
            for _key in self._KEY_MAP:
                self._SERIAL.write(bytearray(_key.lower(), 'utf-8'))
            os._exit(1)

    # [NEEDS helper function]: # _handle_special_keys()
    def on_press(self, key):
        try:
            key = key.char
            _key = self._KEY_MAP[key.upper()]

            #self._SERIAL.write(bytearray(_key.upper(), 'utf-8'))
            if _key not in self._HELD_KEYS:
                self._HELD_KEYS.append(_key)
                self._SERIAL.write(bytearray(_key.upper(), 'utf-8'))
                #print(_key.upper())
            # ^Can use this approach to not send true over and over
        except Exception as e:
            self._last_typed = 0 #time.time()

            if type(key) == kb.Key:
                if key == kb.Key.space:
                    _key = self._KEY_MAP[' ']
                    if _key not in self._HELD_KEYS:
                        self._HELD_KEYS.append(_key)
                else:
                    print('[UNMAPPED KEY {0} PRESSED]'.format(key))
                    # ^DEBUGG
            else:
                print('EXCEPTION: {0}'.format(e))

            self._handle_special_press(key)

    def _handle_special_press(self, key):
        if type(key) == kb.Key:
            if key == kb.Key.space:
                _key = self._KEY_MAP[' ']
                self._HELD_KEYS.remove(_key)

    def on_release(self, key):
        self._handle_special_release(key)

        try:
            key = key.char
            _key = self._KEY_MAP[key.upper()]
            self._HELD_KEYS.remove(_key)
            self._SERIAL.write(bytearray(_key.lower(), 'utf-8'))
        except:
            pass

    # [Special conditions on release]:
    def _handle_special_release(self, key):
        pass
        # [Ignore kb.KeyCode]:
        #if type(key) == kb.Key:
        #    if key == kb.Key.alt:
        #        self._hold_ALT = False

    def on_click(self, x, y, button, pressed):
        _int_x = int(x)
        _int_y = int(y)

        if button==Button.left: # LEFT_CLICK
            #print('LEFT_CLICK: (Fire)')
            if pressed:
                _key = self._KEY_MAP['J']
                if _key not in self._HELD_KEYS:
                    self._HELD_KEYS.append(_key)
                    self._SERIAL.write(bytearray(_key.upper(), 'utf-8'))
            else:
                _key = self._KEY_MAP['J']
                self._HELD_KEYS.remove(_key)
                self._SERIAL.write(bytearray(_key.lower(), 'utf-8'))
        elif button==Button.right: # RIGHT_CLICK
            #print('RIGHT_CLICK: (Sub-weapon)')
            if pressed:
                _key = self._KEY_MAP['I']
                if _key not in self._HELD_KEYS:
                    self._HELD_KEYS.append(_key)
                    self._SERIAL.write(bytearray(_key.upper(), 'utf-8'))
            else:
                _key = self._KEY_MAP['I']
                self._HELD_KEYS.remove(_key)
                self._SERIAL.write(bytearray(_key.lower(), 'utf-8'))
        else:
            print('OTHER_CLICK')
            # RE-CENTER VIEW?

    def program_clock(self, _delay):
        if self._last_moved == 0:
            self._last_moved = time.time()
        else:
            if (time.time() - self._last_moved >= _delay): # After .1 second delay, stop movement direction of camera: (hopefully)
                self._last_moved = time.time()

                # [Every Second send all held keys]:
                #if self._SERIAL and self._last_key is not None:
                #    self._SERIAL.write(bytearray('q', 'utf-8'))
                #    self._SERIAL.write(bytearray('e', 'utf-8'))

    def on_move(self, x, y):
        _int_x = int(x)
        _int_y = int(y)
        self._last_moved = time.time()
        #print('X: {0} | Y: {1}'.format(_int_x, _int_y))

        if self._last_int_x is not None:
            _diff_x = abs(self._last_int_x - _int_x)

        '''
        # [Mouse Controlls]:
        if self._last_int_x:
            # [X]:
            if self._last_int_x > _int_x:
                if _diff_x < 20:
                    _diff_x = '3'
                elif _diff_x < 100: 
                    _diff_x = '1'
                else:
                    _diff_x = '5'
                #print('{0}| LOOKING LEFT'.format(_diff))
            elif self._last_int_x < _int_x:
                if _diff_x < 20:
                    _diff_x = '7'
                elif _diff_x < 100: 
                    _diff_x = '9'
                else:
                    _diff_x = '5'
                #print('{0}| LOOKING RIGHT'.format(_diff))

            if int(_diff_x) > 0:
                print(_diff_x)
                self._SERIAL.write(bytearray(_diff_x, 'utf-8'))
        '''


        self.CHECK_MOUSE_EMERGENCY(_int_x, _int_y)
        self._last_int_x = int(x)
        self._last_int_y = int(y)

    def on_scroll(self, x, y, dx, dy):
        _int_x = int(x)
        _int_y = int(y)
        _int_dx = int(dx)
        _int_dy = int(dy)

        self.CHECK_MOUSE_EMERGENCY(_int_x, _int_y)

        #print('scroll|{0}'.format(_int_dy))