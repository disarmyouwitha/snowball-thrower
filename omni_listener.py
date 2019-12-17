import os
import sys
import time
import pynput.mouse as ms
import pynput.keyboard as kb
from pynput.mouse import Button, Controller
from pynput.keyboard import Key, Controller

class omni_listener():
    # [CONTROLLERS]:
    _ms_ctrl = None # (Because pyautogui can't click to a non-active window)
    _kb_ctrl = None # (Might replace with pyautogui keyUp/keyDown so we don't have to load this controller.. need to check support on osx/win)

    # [MOUSE GLOBALS]:
    _last_click = 0
    _last_int_x = None
    _last_int_y = None
    _SERIAL = False

    # [KEYBOARD GLOBALS]:
    _last_typed = 0
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

    # [Mouse position at 0,0 is emergency exit condition]:
    def CHECK_MOUSE_EMERGENCY(self, _int_x, _int_y):
        if _int_x == 0 and _int_y == 0:
            print('[MOUSE_PANIC_EXIT]')
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
            _key = key.char
            self._last_typed = time.time()
            print(_key)

            # [Send UPPER on key press]:
            if self._SERIAL:
                ser.write(bytearray(key.char.upper(), 'utf-8'))
                line = ser.readline()
                print(line)

            # [If _held_keys treat as char in sequence]:
            _held_keys = self._held_keys()
            _held_cnt = len(_held_keys)
            if _held_cnt >= 2:
                print('HELD_KEYS')

        except AttributeError: 
            print('special key {0} pressed'.format(key))
            # ^DEBUGG

            self._last_typed = 0 #time.time()

            # [If ESC key pressed]:
            if key == kb.Key.esc:
                self._hold_ESC = True
                print('[Listeners "stopped"]')

            self._handle_special_press(key)

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
            print(key.char)
            if self._SERIAL:
                ser.write(bytearray(key.char.lower(), 'utf-8'))
                line = ser.readline()
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

        if pressed:
            self._last_int_x = int(x)
            self._last_int_y = int(y)

        if pressed:
            print('[PRESSED!]')

        # [Determine left/right click]:
        if button==Button.left:
            print('LEFT_CLICK')
        elif button==Button.right:
            print('RIGHT_CLICK')
        else:
            print('OTHER_CLICK')

            # [SAME = CLICK | DIFF = BOX]:
            if abs(_int_x-self._last_int_x) < 5 and abs(_int_y-self._last_int_y) < 5:
                # [Keep track of last click / tell difference between single/double click]:
                if self._last_click == 0:
                    self._last_click = time.time()
                    _click_cnt = 1
                else:
                    if (time.time() - self._last_click) >= .5:
                        _click_cnt = 1
                        self._last_click = time.time()
                        print('SINGLE_CLICK')
                    else:
                        _click_cnt = 2
                        self._last_click = 0
                        print('DOUBLE_CLICK')
            '''
            else:
                if _int_x > self._last_int_x:
                    _which_box = 'ssim-box'
                else:
                    _which_box = 'drag-box'
            '''

    def on_move(self, x, y):
        _int_x = int(x)
        _int_y = int(y)

        self.CHECK_MOUSE_EMERGENCY(_int_x, _int_y)

    def on_scroll(self, x, y, dx, dy):
        _int_x = int(x)
        _int_y = int(y)
        _int_dx = int(dx)
        _int_dy = int(dy)

        self.CHECK_MOUSE_EMERGENCY(_int_x, _int_y)

        print('scroll|{0}'.format(_int_dy))