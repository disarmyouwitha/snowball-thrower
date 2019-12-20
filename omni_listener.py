import os
import sys
import time
import serial
import contextlib
import pynput.mouse as ms
import pynput.keyboard as kb
from pynput.mouse import Button, Controller
from pynput.keyboard import Key, Controller

# Can write config / translation here.. WASD is actual hard-coded values we should send..
# Eventually we can get rid of all of these pins and communicate (Computer)<->Serial<->Serial<->(Switch) 
# and we can implement the Joystick.c logic at this level (or higher) sending only basic ReportData commands to the switch (up, down, left, etc)

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
    _SERIAL = True #False

    # [CONTROLLERS]:
    _ms_ctrl = None # (Because pyautogui can't click to a non-active window)
    _kb_ctrl = None # (Might replace with pyautogui keyUp/keyDown so we don't have to load this controller.. need to check support on osx/win)

    # [MOUSE GLOBALS]:
    _last_moved = 0
    _last_click = 0
    _last_int_x = None
    _last_int_y = None

    # [KEYBOARD GLOBALS]:
    _KEY_MAP = None
    _last_typed = 0
    _last_key = None
    _hold_ESC = False
    _hold_ALT = False
    _hold_TAB = False
    _hold_CMD = False
    _hold_CTRL = False
    _hold_SHIFT = False

    def __init__(self):
        self._kb_ctrl = kb.Controller()
        self._ms_ctrl = ms.Controller()

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

        if self._SERIAL:
            try:
                self._SERIAL = serial.Serial('/dev/cu.usbmodem1421')  # open serial port
            except:
                self._SERIAL = serial.Serial('/dev/cu.usbmodem1411')  # open serial port

    # [Mouse position at 0,0 is emergency exit condition]:
    def CHECK_MOUSE_EMERGENCY(self, _int_x, _int_y):
        if _int_x == 0 and _int_y == 0:
            print('[MOUSE_PANIC_EXIT]')

            # Send LOW for all pins when exiting:
            for _key in self._KEY_MAP:
                if self._SERIAL:
                    self._SERIAL.write(bytearray(_key.lower(), 'utf-8'))
                    line = self._SERIAL.readline()
                    print(line)
            os._exit(1)

    def CLICK(self, which_click='left', num_clicks=1, type_click='click'):
        if type_click=='click':
            if which_click == 'right-click':
                self._ms_ctrl.click(ms.Button.right, num_clicks)
            else:
                self._ms_ctrl.click(ms.Button.left, num_clicks)
        elif type_click=='press':
            if which_click == 'right-click':
                self._ms_ctrl.press(ms.Button.right)
            else:
                self._ms_ctrl.press(ms.Button.left)
        elif type_click=='release':
            if which_click == 'right-click':
                self._ms_ctrl.release(ms.Button.right)
            else:
                self._ms_ctrl.release(ms.Button.left)

    def PRESS(self, key):
        self._kb_ctrl.press(key)
        self._handle_special_press(key)

    def RELEASE(self, key):
        self._kb_ctrl.release(key)
        self._handle_special_release(key)

    # [NEEDS helper function]: # _handle_special_keys()
    def on_press(self, key):
        try:
            #self._last_typed = time.time()
            key = key.char
            _key = self._KEY_MAP[key.upper()]
            #print('{0} => {1}'.format(key, _key))

            # [Send UPPER on key press]:
            if self._SERIAL:
                self._SERIAL.write(bytearray(_key.upper(), 'utf-8'))
                line = self._SERIAL.readline()
                print(line)
        except Exception as e:
            self._last_typed = 0 #time.time()

            if key == kb.Key.space:
                _key = self._KEY_MAP[' ']
                #print('{0} => {1}'.format(key, _key))

                # [Send UPPER on key press]:
                if self._SERIAL:
                    self._SERIAL.write(bytearray(_key.upper(), 'utf-8'))
                    line = self._SERIAL.readline()
                    print(line)
            else:
                print('[UNMAPPED KEY {0} PRESSED]'.format(key))
                # ^DEBUGG

            #self._handle_special_press(key)

    def _handle_special_press(self, key):
        _pass_thru = False

        # [If ALT key pressed]: (Don't accept HELD tab as multiple presses)
        if key == kb.Key.alt:
            if self._hold_ALT == False:
                self._hold_ALT = True
                _pass_thru = True

        # [If CMD key pressed]: (Don't accept HELD tab as multiple presses)
        if key == kb.Key.cmd:
            if self._hold_CMD == False:
                self._hold_CMD = True
                _pass_thru = True

        # [If SHIFT is pressed]: (Don't accept HELD tab as multiple presses)
        if key == kb.Key.shift:
            if self._hold_SHIFT == False:
                self._hold_SHIFT = True
                _pass_thru = True

        # [If CTRL key pressed]: (Don't accept HELD tab as multiple presses)
        if key == kb.Key.ctrl:
            if self._hold_CTRL == False:
                self._hold_CTRL = True
                _pass_thru = True

        # [If ENTER key pressed]: 
        if key == kb.Key.enter:
            _pass_thru = True

        # [If TAB key pressed]: 
        if key == kb.Key.tab:
            if self._hold_TAB == False:
                self._hold_TAB = True
                _pass_thru = True

        # [If ARROW key pressed]: (Held keys?)
        if key == kb.Key.up:
            _pass_thru = True
        if key == kb.Key.down:
            _pass_thru = True
        if key == kb.Key.left:
            _pass_thru = True
        if key == kb.Key.right:
            _pass_thru = True

        '''
        _held_keys = self._held_keys()
        _held_cnt = len(_held_keys)

        # [Remove self from _held_keys]:
        if key in _held_keys:
            _held_keys.remove(key)
        '''


    # [Check HELD status of keys]:
    def _held_keys(self):
        _key_list = []

        if self._hold_ESC:
            _key_list.append(kb.Key.esc)
        if self._hold_ALT:
            _key_list.append(kb.Key.alt)
        if self._hold_TAB:
            _key_list.append(kb.Key.tab)
        if self._hold_CMD:
            _key_list.append(kb.Key.cmd)
        if self._hold_CTRL:
            _key_list.append(kb.Key.ctrl)
        if self._hold_SHIFT:
            _key_list.append(kb.Key.shift)

        return _key_list 

    def on_release(self, key):
        self._handle_special_release(key)

        try:
            key = key.char
            _key = self._KEY_MAP[key.upper()]

            #print(_key.lower())

            if self._SERIAL:
                self._SERIAL.write(bytearray(_key.lower(), 'utf-8'))
                line = self._SERIAL.readline()
                print(line)
        except:
            pass


    # [Special conditions on release]:
    def _handle_special_release(self, key):
        # [Ignore kb.KeyCode]:
        if type(key) == kb.Key:
            if key == kb.Key.alt:
                self._hold_ALT = False
            if key == kb.Key.tab:
                self._hold_TAB = False
            if key == kb.Key.shift:
                self._hold_SHIFT = False
            if key == kb.Key.ctrl:
                self._hold_CTRL = False
            if key == kb.Key.esc:
                self._hold_ESC = False
            if key == kb.Key.cmd:
                self._hold_CMD = False

    def on_click(self, x, y, button, pressed):
        _int_x = int(x)
        _int_y = int(y)

        # [Determine left/right click]:
        if button==Button.left:
            print('LEFT_CLICK: (Fire)')
            # [Determine click/release]:
            if pressed:
                _key = self._KEY_MAP['J']
                #print('{0}| FIRE ON!'.format(_key))
                if self._SERIAL:
                    self._SERIAL.write(bytearray(_key.upper(), 'utf-8'))
                    line = self._SERIAL.readline()
                    print(line)
            else:
                _key = self._KEY_MAP['J']
                #print('{0}| FIRE OFF!'.format(_key))
                if self._SERIAL:
                    self._SERIAL.write(bytearray(_key.lower(), 'utf-8'))
                    line = self._SERIAL.readline()
                    print(line)
        elif button==Button.right:
            print('RIGHT_CLICK: (Sub-weapon)')
            # [Determine click/release]:
            if pressed:
                _key = self._KEY_MAP['U']
                if self._SERIAL:
                    self._SERIAL.write(bytearray(_key.upper(), 'utf-8'))
                    line = self._SERIAL.readline()
                    print(line)
            else:
                _key = self._KEY_MAP['U']
                if self._SERIAL:
                    self._SERIAL.write(bytearray(_key.lower(), 'utf-8'))
                    line = self._SERIAL.readline()
                    print(line)
        else:
            print('OTHER_CLICK')
            # RE-CENTER VIEW?

    def program_clock(self):
        if self._last_moved == 0:
            self._last_moved = time.time()
        else:
            if (time.time() - self._last_moved >= .1): # After .1 second delay, stop movement direction of camera: (hopefully)
                self._last_moved = time.time()

                if self._SERIAL and self._last_key is not None:
                    print('SERIAL SEND: ')
                    _key = self._KEY_MAP['Q']
                    self._SERIAL.write(bytearray(_key.lower(), 'utf-8'))
                    line = self._SERIAL.readline()
                    print(line)
                    _key = self._KEY_MAP['E']
                    self._SERIAL.write(bytearray(_key.lower(), 'utf-8'))
                    line = self._SERIAL.readline()
                    print(line)
                    self._last_key = None
            elif (time.time() - self._last_moved) < 3:
                pass
                #print('last_moved updated')

    def on_move(self, x, y):
        _int_x = int(x)
        _int_y = int(y)
        self._last_moved = time.time()
        #print('X: {0} | Y: {1}'.format(_int_x, _int_y))

        if self._last_int_x is not None:
            _diff = abs(self._last_int_x - _int_x)
            if _diff < 20:
                print('_diff (little): {0}'.format(_diff))
            elif _diff < 100: 
                print('_diff (big): {0}'.format(_diff))

        # [Mouse Controlls]:
        if self._last_int_x:
            # [X]:
            if self._last_int_x > _int_x:
                _key = self._KEY_MAP['Q']
                #print('{0}| LOOKING LEFT'.format(_key))
            elif self._last_int_x < _int_x:
                _key = self._KEY_MAP['E']
                #print('{0}| LOOKING RIGHT'.format(_key))

            try:
                # [Report key for X movement]:
                if _key is not None:
                    self._last_key = _key
                    if self._SERIAL:
                        self._SERIAL.write(bytearray(_key.upper(), 'utf-8'))
                        line = self._SERIAL.readline()
                        #print(line)
            except:
                pass

            '''
            # [Y]:
            if self._last_int_y > _int_y:
                _key = self._KEY_MAP['I']
                #print('{0}| LOOKING UP'.format(_key))
            elif self._last_int_y < _int_y:
                _key = self._KEY_MAP['K']
                #print('{0}| LOOKING DOWN'.format(_key))
            else:
                _key = None

            # [Report key for Y movement]:
            if _key is not None:
                if self._SERIAL:
                    self._SERIAL.write(bytearray(_key.upper(), 'utf-8'))
                    line = self._SERIAL.readline()
                    print(line)
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