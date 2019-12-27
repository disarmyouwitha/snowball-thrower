## [A start]:
I've been watching these repos for a while, finally got myself a Teensy2.0++ and hopped into things. =] 

I've enjoyed the splat-post printer (https://github.com/LightningStalker/Splatmeme-Printer) and I was even further inspired by the snowball-thrower bertrandom wrote (https://github.com/bertrandom/snowball-thrower) as well as splat-booya, an earlier attempt at a Splatoon2 bot by prolific splatoon coder 3096 (https://github.com/3096/Splat-Booyah) 

## [Early testing vid]:
https://www.reddit.com/r/Saltoon/comments/eawfdy/neat_or_nah/

## [Early Mouse Controls]:
https://www.youtube.com/watch?v=vGVgkbJxNNc


## [Splat Bot]:
I have written 2 variants of this code.. one a static bot that follows a set of instructions when you plug it in (much like the Zelda snowball-thrower) but more fleshed out with all of the options it needs to play splatoon. (I will most certainly circle back on this idea at a later date, and will add options to Joystick.hex soon boot into this mode)..

## [Splat Keyboard]:??
The other is a side-side project, where I have set up a spare micro-controller (PHOTON) of mine to receive keyboard input over the serial line with a simple python program.. It communicates over Tx/Rx (serial) to control what inputs the Teensy sends. 

This allows me to send input from a program running on my computer, to the tiny computer pretending to be the switch controller. =] At this point I have mapped 15 of the buttons to keys, and I am able to play Turf Wars via my keyboard (Poorly.. I feel. xD But I think that has a lot to do with no real Mouse support) 

> CURRENT ISSUE: I can controll RX, RY (looking left, right; Looking up, down) with the mouse now! It's.. mostly a software issue holding progress back at this point.. I can't seem to program mouse controls that "feel right" or even "work well enough" to Turf War, I feel.

> ~NEXT UP: Fire gun with click~ I have this done

> ~NEXT UP: Record actions.~ I have this done as well!

> NEXT UP: Playback actions. ** I have this done.. but the timing is off. I need to reimplement on the Teensy.

> ^ THESE STEPS (Record/Replay)should allow us to "bot" more things much more quickly than Guess/Check methods used to instruct the Teensy before. I hope, anyways.

#### [HOW TO USE]: (splat_keyboard)
> 0. Wire PHOTON Tx pin to TEENSY Rx pin; Wire PHOTON Rx pin to TEENSY Tx pin.
> 1. Flash the .ino files to the PHOTON (usb_report)
> 2. Make / Flash the Joystick.hex file to the TEENSY
> 3. run `sudo python3 usb_interface.py` from your computer, it will start to capture your keyboard input

#### [HOW TO USE]:
For compiling and flashing `Joystick.hex` (the main program) follow the instructions below. This will be all you will need to do, aside from hook it into your switch, if you are running the bot that is just "following instructions". For how to use the keyboard interface, read above^^

#### Compiling and Flashing onto the Teensy 2.0++
Go to the Teensy website and download/install the [Teensy Loader application](https://www.pjrc.com/teensy/loader.html). For Linux, follow their instructions for installing the [GCC Compiler and Tools](https://www.pjrc.com/teensy/gcc.html). For Windows, you will need the [latest AVR toolchain](http://www.atmel.com/tools/atmelavrtoolchainforwindows.aspx) from the Atmel site. See [this issue](https://github.com/LightningStalker/Splatmeme-Printer/issues/10) and [this thread](http://gbatemp.net/threads/how-to-use-shinyquagsires-splatoon-2-post-printer.479497/) on GBAtemp for more information. (Note for Mac users - the AVR MacPack is now called AVR CrossPack. If that does not work, you can try installing `avr-gcc` with `brew`.)

Next, you need to grab the LUFA library. You can download it in a zipped folder at the bottom of this page. Unzip the folder, rename it LUFA, and place it where you like. Then, download or clone the contents of this repository onto your computer. Next, you'll need to make sure the LUFA_PATH inside of the makefile points to the LUFA subdirectory inside your LUFA directory. My Switch-Fightstick directory is in the same directory as my LUFA directory, so I set LUFA_PATH = ../LUFA/LUFA.

Now you should be ready to rock. Open a terminal window in the `splat_bot` directory, type make, and hit enter to compile. If all goes well, the printout in the terminal will let you know it finished the build! Follow the directions on flashing `Joystick.hex` onto your Teensy, which can be found page where you downloaded the Teensy Loader application.

#### Thanks
> I stand on the shoulders of GIANTS: TY to everyone from the person who worked on LUFA to the person who reversed the HORI Fightstick to the meme'rs who set up the first splat-post printer, inspiring many projects from there. =]

> Thanks to bertrandom https://github.com/bertrandom/snowball-thrower for his work on adding LOGOs to the existing code and setting up a more structural way to communicate with the switch. (https://medium.com/@bertrandom/automating-zelda-3b37127e24c8)

> Thanks to Shiny Quagsire for his [Splatoon post printer](https://github.com/shinyquagsire23/Switch-Fightstick) and progmem for his [original discovery](https://github.com/progmem/Switch-Fightstick).
