## splat_bot:
I've been watching these repos for a while, finally got myself a Teensy2.0++ and hopped into things. =] 

I've enjoyed the splat-post printer, and I was even further inspired by the snowball thrower bertrandom wrote 
(as well as splat-booya an earlier attempt at a Splatoon2 bot!)

More updates to come?

#### How to use
    Not sure yet!

This repository has been tested using a Teensy 2.0++.

#### Compiling and Flashing onto the Teensy 2.0++
Go to the Teensy website and download/install the [Teensy Loader application](https://www.pjrc.com/teensy/loader.html). For Linux, follow their instructions for installing the [GCC Compiler and Tools](https://www.pjrc.com/teensy/gcc.html). For Windows, you will need the [latest AVR toolchain](http://www.atmel.com/tools/atmelavrtoolchainforwindows.aspx) from the Atmel site. See [this issue](https://github.com/LightningStalker/Splatmeme-Printer/issues/10) and [this thread](http://gbatemp.net/threads/how-to-use-shinyquagsires-splatoon-2-post-printer.479497/) on GBAtemp for more information. (Note for Mac users - the AVR MacPack is now called AVR CrossPack. If that does not work, you can try installing `avr-gcc` with `brew`.)

LUFA has been included as a git submodule, so cloning the repo like this:

```
git clone --recursive git@github.com:bertrandom/snowball-thrower.git
```

will put LUFA in the right directory.

Now you should be ready to rock. Open a terminal window in the `snowball-thrower` directory, type `make`, and hit enter to compile. If all goes well, the printout in the terminal will let you know it finished the build! Follow the directions on flashing `Joystick.hex` onto your Teensy, which can be found page where you downloaded the Teensy Loader application.

#### Thanks
> I stand on the shoulders of GIANTS: TY to everyone from the person who worked on LUFA to the person who reversed the HORI Fightstick to the meme'rs who set up the first splat-post printer, inspiring many projects from there. =]

> Thanks to bertrandom https://github.com/bertrandom/snowball-thrower for his work on adding LOGOs to the existing code and setting up a more structural way to communicate with the switch. (https://medium.com/@bertrandom/automating-zelda-3b37127e24c8)

> Thanks to Shiny Quagsire for his [Splatoon post printer](https://github.com/shinyquagsire23/Switch-Fightstick) and progmem for his [original discovery](https://github.com/progmem/Switch-Fightstick).

> Thanks to [exsilium](https://github.com/bertrandom/snowball-thrower/pull/1) for improving the command structure, optimizing the waiting times, and handling the failure scenarios. It can now run indefinitely!