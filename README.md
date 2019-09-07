<p align="center"><a href="https://github.com/wholder/TeensyCNC2/blob/master/images/TeensyMod.jpg"><img src="https://github.com/wholder/TeensyCNC2/blob/master/images/TeensyMod.jpg" width="75%" height=75%"></a></p>

## TeensyCNC2

TeensyCNC2 is an updated version of [TeensyCNC](https://github.com/seishuku/TeensyCNC), which is a program created by Matt Williams that
runs on a Teensy 3.2 board that's been retrofitted into a modified Cricut™ Mini in order to make it useful again.  In an amazing display
of corporate tone deafness, Provo Craft, the original makers of the Cricut™ Mini decided in 2018  to make it obsolete by shutting down
the "[Craft Room](http://inspiration.cricut.com/cricut-craft-room-closing/)" on-line service needed to make the Cricut™ Mini function and
leaving current owners, such as Matt's wife, unable to use it.  Prior to this, Provo Craft had also sued and shut down at two makers of
3rd party software intended to work with their products.  And, as a middle finger to device hackers, use a form of encrypted communication to discourage anyone from trying to create software that can commnicate with their products.

However, Matt's ingenious "hack" completely bypasses all of Provo Craft's sneaky attempts to control how users can use
their products by replacing the two PIC microcontollers inside the Cricut Mini with an inexpensive, Arm-based microcontroller board, the
[Teensy 3.2](https://www.pjrc.com/store/teensy32.html).  I came across Matt's work while I was working on my 
[LaserCut](https://github.com/wholder/LaserCut) program, which is Java-based program designed to allow users to create 2D vector
designs and then send them to laser, paper or vinyl cutters.  While Matt's original code worked fine, I decided to rework it on order to make a few changes and improvements, such as:
 
  - Rewriting the G-Code parser using a state machine to make it easier to extend and revise
  - Reversing the Y axis so that the origin of the cutting mat is the upper left rather than the lower left
  - Rework the button press code and add wiring to allow use of the Power button and lEDs on the control panel

## Fight Back against Forced Obsolence!

If you are reasonably skilled with using a screwdriver and a soldering iron, you should be able to easily modify a Cricut Mini in a few hours
and turn an otherwise useless device back into something fun and useful.  But, first, you'll need to obtain the following required materials and tools,
or their equivalent:
   - A Cricut Mini (model CMNI001 with Power Supply) (1)
   - Adjustable, Temperature-controlled Soldering Iron
   - Medium size Philips screwdriver
   - [Teensy 3.2 Board without presoldered pins](https://www.pjrc.com/store/teensy32.html) (Note 2)
   - [Rosin Core, 60/40 solder](https://www.adafruit.com/product/1886)
   - [Copper solder wick](https://www.adafruit.com/product/149)
   - [30 gauge, tin plated, Kynar™ insulated "wire wrap" wire](https://www.adafruit.com/product/1446) (Note 3)
   - [Precision, 20-30 gauge wire strippers](https://www.adafruit.com/product/527)
   - [Double-sided, 1 inch foam tape](https://www.amazon.com/gp/product/B00347A8GC/)
   
Note 1: Because it's now orphaned by Provo Craft, I was able to purchase a new, unused Cricut Mini for $30 on Craigslist.  They are usually quite a few of them available on eBay, but often for ridiculously high prices, so you made need to be patient to find one there at a reasonable price.  You might also find them at local swap meets and garage sales.  If you find one missing its power supply, you can substitute an 18 Volt DC laptop power supply that can supply at least 1.8Amps and has a center positive, 5.5 mm OD, 2.5 mm ID Barrel Jack.

Note 2: The discontinued [Teensy 3.1](https://www.pjrc.com/teensy/teensy31.html) can also be used, as it is pin compatible with the Teensy 3.2.

Note 3: Kynar insulated, 30 gauge wire wrap wire is strongly recommended because it's tough and because the Kynar (polyvinylidene fluoride)
insulation won't shrink or melt under typical soldering conditions.

The process involves the following steps:

 1. Use the Philips screwdriver to open the Cricut Mini's case to get access to its printed circuit control board (PCB). [Click here](markdown/disassembly.md) to see a visual guide on how to disasmble the Cricut Mini. 
 2. Unsolder and remove the 14 pin and 28 pin "PIC" microprocesors from the PCB.  [Click here](images/removethese.jpg) to see where these are located on the PCB.  Alternatively, you could use small diagonal cutters to carefully clip each pin of the IC package to remove the IC and then use the soldering iron to remove the leftover pins.  However, be careful not to damage or nick the delicate traces on the PCB.
 3. Use solder wick to clean up the pads and remove the old solder, then use the Sn63/Pb37 solder to tin each of the pads indicated with arrows [in this photo](images/solderpads.jpg). At the same time, carefully  unsolder resistor R7 shown in the same photo and tin the pad on the left of the PCB.
 4. Download the file [main.hex](https://github.com/wholder/TeensyCNC2/blob/master/main.hex) and use it to [program the Teensy](https://www.pjrc.com/teensy/first_use.html).  Depending on which operating system you use, you'll first need to download the [Teensy Loader](https://www.pjrc.com/teensy/loader.html).
 5. Attach the Teensy to the MIni's PCB using some double-sided foam tape, [as shown in this photo](images/foamtape.jpg).
 6. 
 

### more coming soon