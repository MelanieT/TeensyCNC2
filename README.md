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
   - Temperature-controlled soldering iron
   - Medium size Philips screwdriver
   - [Teensy 3.2 Board without presoldered pins](https://www.pjrc.com/store/teensy32.html) (Note 1)
   - [Rosin Core, Sn63/Pb37 solder](https://www.amazon.com/Solder-Diameter-Storage-Welding-Soldering/dp/B01N0VNNKO/)
   - [Copper solder wick](https://www.amazon.com/NTE-Electronics-SW02-10-No-Clean-Blue-098/dp/B0195UVWJ8/)
   - [30 gauge, tin plated, Kynar™ insulated "wire wrap" wire](https://www.amazon.com/gp/product/B006C4ARR4/) (Note 2)
   - Precision, 30 gauge wire strippers, such as the [Jonard ST-500](https://www.amazon.com/Jonard-ST-500-Adjustable-Precision-Thickness/dp/B001ICLVN4/) (Note 3)
   - [Double-sided, 1 inch foam tape](https://www.amazon.com/gp/product/B00347A8GC/)
   
Note 1: The discontinued [Teensy 3.1](https://www.pjrc.com/teensy/teensy31.html) can also be used, as it is pin compatible with the Teensy 3.2.

Note 2: Kynar insulated, 30 gauge wire wrap wire is strongly recommended because it's tough and because the Kynar (polyvinylidene fluoride)
insulation won't shrink or melt under typical soldering conditions.

Note 3: at $38, the Jonard ST-500 is a bit on the pricey side.  But, in my experience, if you like to build electronic projects, it's a
tool worth investing in.  However, if you use it carefully, a less expensive, wire stripping tool, such as the
[Hakko CHP CSP-30-1](https://www.amazon.com/Hakko-CSP-30-1-Stripper-Maximum-Capacity/dp/B00FZPHMUG/ref=sr_1_4) can also produce good results.


The process involves the following steps:

 1. Use the Philips screwdriver to open the Cricut Mini's case to get access to its printed circuit control board (PCB). [Click here](markdown/disassembly.md) to see a visual guide on how to disasmble the Cricut Mini. 
 2. Unsolder and remove the 14 pin and 28 pin "PIC" microprocesors from the PCB.  [Click here](images/removethese.jpg) to see where these are located on the PCB.  Alternatively, you could use small diagonal cutters to carefully clip each pin of the IC package to remove the IC and then use the soldering iron to remove the leftover pins.  However, be careful not to damage or nick the delicate traces on the PCB.
 3. Use solder wick to clean up the pads and remove the old solder, then use the Sn63/Pb37 solder to tin each of the pads indicated with arrows [in this photo](images/solderpads.jpg). At the same time, carefully  unsolder resistor R7 shown in the same photo and tin the pad on the left of the PCB.
 4. Download the file [main.hex](https://github.com/wholder/TeensyCNC2/blob/master/main.hex) and use it to [program the Teensy](https://www.pjrc.com/teensy/first_use.html).  Depending on which operating system you use, you'll first need to download the [Teensy Loader](https://www.pjrc.com/teensy/loader.html).
 5. Attach the Teensy to the MIni's PCB using some double-sided foam tape, [as shown in this photo](images/foamtape.jpg).
 6. 
 

### more coming soon