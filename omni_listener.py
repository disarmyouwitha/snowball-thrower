import os
import sys
import mss
import time
import serial
import pynput.mouse as ms
import pynput.keyboard as kb
from pynput.mouse import Button, Controller
from pynput.keyboard import Key, Controller

class omni_listener():
    # [CONTROLLERS]:
    _ms_ctrl = None
    _kb_ctrl = None

    # [SCREEN GLOBALS]:
    _width = 0
    _height = 0
    _SERIAL = None

    # [MOUSE GLOBALS]:
    _last_moved = 0
    _last_int_x = None
    _last_int_y = None
    _last_checked_x = None
    _last_checked_y = None

    # [KEYBOARD GLOBALS]:
    _KEY_MAP = None
    _HELD_KEYS = []
    _HELD_TIMERS = {}
    _ACTION_LIST = []
    _START_CLOCK = time.time()

    def __init__(self):
        self._kb_ctrl = kb.Controller()
        self._ms_ctrl = ms.Controller()

        self.mouse_listener = ms.Listener(on_click=self.on_click, on_move=self.on_move, on_scroll=self.on_scroll)
        self.keyboard_listener = kb.Listener(on_press=self.on_press, on_release=self.on_release)
        # ^(non-blocking mouse/keyboard listener)

        # Get screen resolution
        with mss.mss() as sct:
            monitor = sct.monitors[1] #0: All | 1: first | 2: second
            self._width = monitor['width']
            self._height = monitor['height']
            self._screen_center = (self._width/2, self._height/2)

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
            'U': 'U',     # U| Camera Reset (Y)
            'M': 'M'      # M| special
        }

        # Connect to serial port (or disable serial if you cant)
        try:
            self._SERIAL = serial.Serial('/dev/cu.usbmodem1421')  # right USB port
        except:
            try:
                self._SERIAL = serial.Serial('/dev/cu.usbmodem1411')  # left USB port
            except:
                print('[Could not connect to serial, disabling]')
                self._SERIAL = False

    # [Mouse position at 0,0 is emergency exit condition]:
    def CHECK_MOUSE_EMERGENCY(self, _int_x, _int_y):
        if _int_x == 0 and _int_y == 0:
            print('[MOUSE_PANIC_EXIT]')

            # Python to output list like this that I can copy into C :3
            print('\r\n[ACTION_LIST]:')
            print('//-------')
            print('_action_length = {0};'.format(len(self._ACTION_LIST)))
            print('//struct action_item action_list[{0}];'.format(len(self._ACTION_LIST)))
            _cnt = 0
            for action in self._ACTION_LIST:
                print("action_list[%s]= (action_item){ '%s', %s, %s };" % (_cnt, *action))
                _cnt+=1
            print('//---')

            # Send LOW for all pins when exiting:
            if self._SERIAL:
                for _key in self._KEY_MAP:
                    self._SERIAL.write(bytearray(_key.lower(), 'utf-8'))
                self._SERIAL.write(bytearray('3', 'utf-8')) # Center camera X
                self._SERIAL.write(bytearray('8', 'utf-8')) # Center camera Y
            os._exit(1)

    def time_action(self, _key):
        if _key not in self._HELD_TIMERS:# or _key == ' ': # space isn't kept in held_keys
            self._HELD_TIMERS[_key] = time.time()
        else:
            _end = time.time()
            _start = self._HELD_TIMERS[_key]
            _time = (_end-_start)
            del self._HELD_TIMERS[_key]
            self._ACTION_LIST.append((_key, '{:04.3f}'.format(_start-self._START_CLOCK), '{:04.3f}'.format(_time)))
            return _time

    # [NEEDS helper function]: # _handle_special_keys()
    def on_press(self, key):
        try:
            key = key.char
            _key = self._KEY_MAP[key.upper()]

            if _key not in self._HELD_KEYS:
                self._HELD_KEYS.append(_key)
                self.time_action(_key)
                if self._SERIAL:
                    self._SERIAL.write(bytearray(_key.upper(), 'utf-8'))
                
                # Reset mouse to center screen when camera is reset:
                if _key == 'R':
                    self._ms_ctrl.position = (self._screen_center)
        except Exception as e:
            if type(key) == kb.Key:
                if key == kb.Key.space:
                    _key = self._KEY_MAP[' ']
                    if _key not in self._HELD_KEYS:
                        self._HELD_KEYS.append(_key)
                        self.time_action(_key)
                        if self._SERIAL:
                            self._SERIAL.write(bytearray(' ', 'utf-8'))
                #else:
                #    print('[UNMAPPED KEY {0} PRESSED]'.format(key))
                #    # ^DEBUGG
            else:
                print('EXCEPTION: {0}'.format(e))

            if type(key) == kb.Key:
                if key == kb.Key.space:
                    _key = self._KEY_MAP[' ']
                    self._HELD_KEYS.remove(_key)

    def on_release(self, key):
        try:
            key = key.char
            _key = self._KEY_MAP[key.upper()]
            self._HELD_KEYS.remove(_key)
            _time = self.time_action(_key)
            if self._SERIAL:
                self._SERIAL.write(bytearray(_key.lower(), 'utf-8'))
        except:
            if type(key) == kb.Key:
                if key == kb.Key.space:
                    _time = self.time_action(' ')
            pass

    def on_click(self, x, y, button, pressed):
        _int_x = int(x)
        _int_y = int(y)

        if button==Button.left: # LEFT_CLICK
            #print('LEFT_CLICK: (Fire)')
            if pressed:
                _key = self._KEY_MAP['J']
                if _key not in self._HELD_KEYS:
                    self._HELD_KEYS.append(_key)
                    if self._SERIAL:
                        self._SERIAL.write(bytearray(_key.upper(), 'utf-8'))
            else:
                _key = self._KEY_MAP['J']
                self._HELD_KEYS.remove(_key)
                if self._SERIAL:
                    self._SERIAL.write(bytearray(_key.lower(), 'utf-8'))
        elif button==Button.right: # RIGHT_CLICK
            #print('RIGHT_CLICK: (Sub-weapon)')
            if pressed:
                _key = self._KEY_MAP['I']
                if _key not in self._HELD_KEYS:
                    self._HELD_KEYS.append(_key)
                    if self._SERIAL:
                        self._SERIAL.write(bytearray(_key.upper(), 'utf-8'))
            else:
                _key = self._KEY_MAP['I']
                self._HELD_KEYS.remove(_key)
                if self._SERIAL:
                    self._SERIAL.write(bytearray(_key.lower(), 'utf-8'))
        else:
            print('OTHER_CLICK')
            # RE-CENTER VIEW?

    def program_clock(self, _delay):
        # Ran after _delay of not moving mouse:
        if self._last_moved == 0:
            self._last_moved = time.time()
        else:
            if (time.time() - self._last_moved >= _delay): # After .1 second delay, stop movement direction of camera: (hopefully)
                self._last_moved = time.time()

                # If Mouse has stopped moving (X) send center-stick:
                '''
                if self._last_int_x:
                    if self._last_checked_x is None:
                        self._last_checked_x = self._last_int_x
                    else:
                        if abs(self._last_checked_x - self._last_int_x) < 20:
                            # Reset Mouse X:
                            x_chars = ['1', '2', '4', '5'] # Don't clear 3
                            for _char in x_chars:
                                try:
                                    self._HELD_KEYS.remove(_char)
                                except:
                                    pass

                            if '3' not in self._HELD_KEYS:
                                print('SENDING CLEAR X') #
                                self._HELD_KEYS.append('3')
                                self._ms_ctrl.position = (self._screen_center)
                                if self._SERIAL:
                                    self._SERIAL.write(bytearray('3', 'utf-8'))

                        # Set _last_checked_x
                        self._last_checked_x = self._last_int_x
                '''

                # If Mouse has stopped moving (Y) send center-stick:
                '''
                if self._last_int_y:
                    if self._last_checked_y is None:
                        self._last_checked_y = self._last_int_y
                    else:
                        if abs(self._last_checked_y - self._last_int_y) < 20:
                            # Reset Mouse Y:
                            y_chars = ['6', '7', '9', '0'] # Don't clear 3
                            for _char in y_chars:
                                try:
                                    self._HELD_KEYS.remove(_char)
                                except:
                                    pass

                            if '8' not in self._HELD_KEYS:
                                print('SENDING CLEAR Y') #
                                self._HELD_KEYS.append('8')
                                self._ms_ctrl.position = (self._screen_center)
                                if self._SERIAL:
                                    self._SERIAL.write(bytearray('8', 'utf-8'))

                        # Set _last_checked_Y
                        self._last_checked_y = self._last_int_y
                '''

    def on_move(self, x, y):
        _int_x = int(x)
        _int_y = int(y)
        self._last_moved = time.time()
        #print('X: {0} | Y: {1}'.format(_int_x, _int_y))

        # Fix weird mouse glitch (_int_x=0 would not reset.. math)
        if _int_x == 0:
            self._ms_ctrl.position = (_int_x+1, _int_y)

        # [Mouse Roadmap]:
        # 1 | -300x = left-side of outer-ring    (MAX_STICK = 255)
        # 2 | -35x  = left-side of center-ring   (MIN_STICK = 0)
        # 3 | 0x (center stick)
        # 4 | +35x = right-side of center-ring
        # 5 | +300x = right-side of outer-ring
        # 6 | -300y = left-side of outer-ring
        # 7 | -35y  = left-side of center-ring
        # 8 | 0y (center stick)
        # 9 | +35y = right-side of center-ring
        # 0 | +300y = right-side of outer-ring
        #-------------------------------------

        # Get distance from center mouse has moved:
        (_center_x, _center_y) = self._screen_center
        _diff_x = (_int_x - _center_x)
        _diff_y = (_int_y - _center_y)

        # [Mouse Controlls]: (Can probably reclaim 2 keys, by not sending char for "center stick"?)
        '''
        if _diff_x <= -300:
            _char_x = '1' # min stick
        elif _diff_x > -300 and _diff_x <-35:
            _char_x = '2'
        elif _diff_x >= -35 and _diff_x <= 35:
            _char_x = '3' # center stick
        elif _diff_x < 300 and _diff_x > 35: 
            _char_x = '4'
        elif _diff_x >= 300:
            _char_x = '5' # max stick

        if _char_x not in self._HELD_KEYS:
            # Reset Mouse X:
            x_chars = ['1', '2', '3', '4', '5']
            for _char in x_chars:
                try:
                    self._HELD_KEYS.remove(_char)
                except:
                    pass

            # send current _char_x:
            self._HELD_KEYS.append(_char_x)
            print('_char_x: {0}'.format(_char_x))
            if self._SERIAL:
                self._SERIAL.write(bytearray(_char_x, 'utf-8'))
        '''

        # [Mouse Controlls]: (Can probably reclaim 2 keys, by not sending char for "center stick"?)
        '''
        if _diff_y <= -300:
            _char_y = '6' # min stick
        elif _diff_y > -300 and _diff_y <-35:
            _char_y = '7'
        elif _diff_y >= -35 and _diff_y <= 35:
            _char_y = '8' # center stick
        elif _diff_y < 300 and _diff_y > 35: 
            _char_y = '9'
        elif _diff_y >= 300:
            _char_y = '0' # max stick

        if _char_y not in self._HELD_KEYS:
            # Reset Mouse Y:
            y_chars = ['6', '7', '8', '9', '0']
            for _char in y_chars:
                try:
                    self._HELD_KEYS.remove(_char)
                except:
                    pass

            # send current _char_x:
            self._HELD_KEYS.append(_char_y)
            print('_char_y: {0}'.format(_char_y))
            if self._SERIAL:
                self._SERIAL.write(bytearray(_char_y, 'utf-8'))
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