<p align="center"><a href="https://github.com/wholder/TeensyCNC2/blob/master/images/cricutmini.jpg"><img src="https://github.com/wholder/TeensyCNC2/blob/master/images/cricutmini.jpg" width="75%" height=75%"></p>

## TeensyCNC2

### Note: this page, and the related code is still under development and will likely change when you least expect it.  This notice will be removed when this project is ready for use.

TeensyCNC2 is an updated version of [TeensyCNC](https://github.com/seishuku/TeensyCNC), which is a program created by Matt Williams. TeensyCNC2 runs on a Teensy 3.2 board that's been retrofitted into a modified Cricut™ Mini (model CMNI001) in order to make it useful again.  In an amazing display of corporate tone deafness, Provo Craft™, the original makers of the Cricut™ Mini decided, in 2018, to make it effectively useless by shutting down the "[Craft Room](http://inspiration.cricut.com/cricut-craft-room-closing/)" on-line service needed to make the Cricut™ Mini function and leaving current owners, such as Matt's wife, unable to use it.  Prior to this, Provo Craft had also sued and shut down at two makers of 3rd party software intended to work with their products and which might have been able to support the Mini.  And, as a middle finger to device hackers looking to find a way to reverse engineer the protocol needed to send designs to a Mini, all of Provo Craft's devices use a form of encrypted communication to discourage anyone from trying to do so.

However, Matt's ingenious "hack" completely bypasses all of Provo Craft's sneaky attempts to control what owner can do with  their legally-owned products  It does this by replacing the two PIC micro-controllers inside the Cricut Mini with an inexpensive, Arm-based micro-controller board called the [Teensy 3.2](https://www.pjrc.com/store/teensy32.html).  I came across Matt's work while I was working on my [LaserCut](https://github.com/wholder/LaserCut) program, which is a Java-based program designed to allow users to create, or import 2D vector designs and then send them to laser, paper or vinyl cutters.  While Matt's original code worked fine, I decided to rework it a bit in order to have a bit of fun, learn about how Matt's code worked and also to make a few changes and improvements, such as:
 
  - Rewriting the G-Code parser using a state machine to make it easier to extend and revise
  - Reversing the Y axis so that the origin of the cutting mat is the upper left rather than the lower left
  - Rework the button press code and add wiring to allow use of the Power button and LEDs on the control panel

### Limitations

This modification does not support the use of any of Provo Craft's proprietary shape and pattern cartridges, as the information in these cartridges is encrypted.

## Fight Back against Forced Obsolesce!

If you are reasonably skilled with using a screwdriver and a soldering iron, you should be able to easily modify a Cricut Mini in a few hours and turn an otherwise useless device back into something fun and useful.  But, first, you'll need to obtain the following required materials and tools, or their equivalent:

   - A Cricut Mini (model CMNI001 with Power Supply) (1)
   - Adjustable, Temperature-controlled Soldering Iron
   - Medium size Philips screwdriver
   - Small diagonal cutters
   - [Teensy 3.2 Board without pre-soldered pins](https://www.pjrc.com/store/teensy32.html) (Note 2)
   - [Rosin Core, 60/40 solder](https://www.adafruit.com/product/1886)
   - [Copper solder wick](https://www.adafruit.com/product/149)
   - [30 gauge, tin plated, Kynar™ insulated "wire wrap" wire](https://www.adafruit.com/product/1446) (Note 3)
   - [Precision, 20-30 gauge wire strippers](https://www.adafruit.com/product/527)
   - [Double-sided, 1 inch foam tape](https://www.amazon.com/gp/product/B00347A8GC/)
   
Note 1: Because it's now orphaned by Provo Craft, I was able to purchase a new, unused Cricut Mini for $30 on Craigslist.  They are usually quite a few of them available on eBay, but often for ridiculously high prices, as sellers don;t seem to have caught on that the Mini is now, effectively useless.  So,you made need to be patient to find one there at a reasonable price.  You might also find them at local swap meets and garage sales, or even at a thrift store.  If you find one missing its power supply, you can substitute an 18 Volt DC laptop power supply that can supply at least 1.8 Amps and has a center positive, 5.5 mm OD, 2.5 mm ID Barrel Jack.  You may also find these in surplus, or thrift stores, or ebay.  If you intend to use your Mini to cut materials, look for one that comes with the Adjustable Blade Holder.

Note 2: The discontinued [Teensy 3.1](https://www.pjrc.com/teensy/teensy31.html) can also be used, as it is pin compatible with the Teensy 3.2.

Note 3: Kynar insulated, 30 gauge wire wrap wire is strongly recommended because it's tough and the Kynar (polyvinylidene fluoride) insulation won't shrink or melt under typical soldering conditions.

## Modifying a Cricut™ Mini

 1. Use the Philips screwdriver to open the Cricut Mini's case to get access to its printed circuit control board (PCB). [Click here](markdown/disassembly.md) to see a visual guide on how to disassemble the Cricut Mini. 
 2. Unsolder and remove the 14 pin and 28 pin "PIC" microprocessors from the PCB.  [Click here](images/removethese.jpg) to see where these are located on the PCB.  This is probably the trickiest part of the conversion, as it normally easiest to use a heat gun to remove surfac emount part.  However, if you are careful, you can use small diagonal cutters to clip each pin of the IC package to remove the IC and then use the soldering iron to remove the leftover pins.  However, be careful not to damage or nick the delicate traces on the PCB.  Alternatively, you might also try [the technique shows in this YouTube video](https://www.youtube.com/watch?v=CVsmwFAkf7I).
 3. After both ICs are removed, use solder wick to clean up the pads and remove the old solder and inspect the PCB carefully to make sure none of the traces were damaged. Then, use the 60/40 solder to tin each of the pads where you going to connect wires, as indicated with arrows [in this photo](images/solderpads.jpg). Add enough solder so you'll have enough on each pad to tack down a wire later.  Finally, carefully  unsolder resistor R7 shown in the same photo and tin the pad on the left of the PCB.
 4. Download the file [main.hex](https://github.com/wholder/TeensyCNC2/blob/master/main.hex) and use it to [program the Teensy](https://www.pjrc.com/teensy/first_use.html).  Depending on which operating system you use, you'll first need to download the [Teensy Loader](https://www.pjrc.com/teensy/loader.html).
 5. Attach the Teensy to the MIni's PCB using some double-sided foam tape, [as shown in this photo](images/foamtape.jpg).  Be sure to orient the Teensy so that the end with the USB connector is nearest to the 2 ICs you removed.  I recommend that use two layers of foam tape and make cutouts in the layer closest to the Teensy for the pushbutton and the USB connector.  Then, cover this with another later to stick onto the PCB.
 6. Once the Teensy is firmly attached, you can starting cutting lengths of wire wrap wire to make the connections needed to connect the Teensy to the solder pads you tinned in step 3.  [Click here to see step by step instructions](markdown/wiring.md) for the wiring connections.
 7. Once you've completed all the wiring connections, I recommend that you use a multimeter set to check continuity with a beep sound and verify all the connections one by one.  Also, use a magnifying glass to examine each pad for possible shorts to adjacent pads.  Pay special attention the connections you made to the Mini's PCB, as those are fairly close together and excess solder could easily bridge one to another.  In fact, just to be sure, use the multimeter to check each adjacent pair for shorts.  You should also use the multimeter to check thet there is not a short from VUSB to Gnd on the Teensy, as that would be very bad.
 8. Once you're confident that all the wiring connections are correct, you can reassemble the Mini by following the [disassembly instructions](markdown/disassembly.md) in reverse and you should then be ready to give it a try.  However, I recommend that you first momentarily carefully connect the Mini's power supply to the modified PCB to make sure nothing starts to smoke.
 9. As a first test, connect the Mini's power supply and also connect the USB cable to either your computer, or to a source of 5 volts (the Teensy is powered from the USB connector and not from the Mini's power supply.)  Once this is done, the "Power" button (the larger, lower button) on the front panel of the Mini should light up green.  Pressing the "load" button once (the small, upper button with up/down arrows on it) should cause the rollers to spin as it tries to load a cutting mat.  Once in the loaded position, the button should light up green.  Pressing the button again should cause the rollers to spin again for a longer period as it tries to unload the cutting mat, after which the button should go dark to indicate the unloaded position.  If you've gotten this far, congratulations!, your Mini should now be ready to use.
 
 ## Sending GCode to a Modified Cricut™ Mini
 
 Matt designed TeensyCNC to accept and process [GCode](https://en.wikipedia.org/wiki/G-code), which is the _lingua franca_ of the CNC world.  My version, TeensyCNC2, is intended to work with my LaserCut program, which is how I recommend you use it.  However, under the hood, TeensyCNC2 still processes GCode, just as Matt's original code did.  To see this in action, you can use a terminal program to connect to your modified Mini and send it GCode commands.  To do this, you'll need to follow the following steps:
 
  1. Make sure your PC can see the Teensy.  In not, depending upon your operating system, you may need to first install a driver (Windows), set up permissions (Linux) before you can use a terminal program to send commands to the Teensy inside your modified Mini.
  2. Teensy always communicates at full USB speed, but set your terminal program to 9600 baud with 8 data bits, no parity and 1 stop bit, just in case.
  3. If not set this way by default, you may also need to configure your terminal program to send either a linefeed (LF), or both a carraige return (CR) and a line feed (LF) when you press Enter" as TeensyCNC2 sees the linefeed (LF) as the signal to process a line of GCode.
 
 Once you have this all set up and you are connected to the Teensy inside the Mini, type "**`G20`**" and press return.  TeensyCNC2 should echo back "**`ok`**" which indicates it has accepted and processed the command.  In this case, **`G20`** tells TeensyCNC2 to begin interpreting all dimensions in "inches" (the default is millimeters and you can set TeensyCNC2 back to millimeters by typing "**`G21`**" and pressing enter.)
 
 You can use the "**`G00`**" or "**`G0`**" command to rapidly move the Mini's cutting, or drawing tool to a new position (with the tool in the raised, non-engaged position.)  For example, to move to position X = 1, Y = 1, you would type "**`G0 X1 Y1`**" and press enter.  Then, to cut or draw, use the "**`G01`**" or "**`G1`**" command to first lower the tool then move with tool engaged.  For example, to cut/draw to position X = 2, Y = 1 type "**`G1 X2 Y1`**" and press enter.  Then, to raise the tool and move it back to the "home" position (X = 0, Y = 0) type "**`G28`**" (home) and press enter.  You can set the speed at which the tool cuts, or draws by adding an "Fn" parameter, where "n" is a value in either inches/minute if you have issued a G20 command, or millimeters/minute if you have issued a G21 command.  The maximum recommended feed rate for the Mini is 100 inches/minute, or 2540 millimeters/minute.  You can append this command to a G1 command, or send it by itself.  Once set, this movement rate will persist until the Mini is reset, or until another Fn value is sent.
 
 ## Using a Modified Cricut™ Mini with LaserCut
 
 To use LaserCut with the Mini, go to the the [LaserCut Project page on GitHub](https://github.com/wholder/LaserCut) and follow the instructions to download and install LaserCut on your operating system.  Once you have LaserCut running and are familiar with how it works, you can configure it to use the Mini as its output device by first selecting "**Preferences**" in the "**File**" menu and then choosing "**Mini Cutter**" as the output device.  This will cause a new menu labelled "**Mini Cutter**" to appear near the righthand side of the menu bar.  You can then use the "**Port**" submenu in the "**Mini Cutter**" menu to select the appropriate serial device.  Once laserCut is connected to the Mini, it will remember the Port setting so you should not have to set it again.  _However, as mentioned in the section above on "_Sending GCode to a Modified Cricut™ Mini_", you may also need to install a serial driver, or set up permissions depending on the requirements of the operating system you are using.)_  
 
 LaserCut should now be ready to work with your modified Mini.  In addition to the Load button on the Mini, can also use the "Load Mat" and "Unload Mat" submenus in the "Mini Cutter" menu to load, or unload the cutting mat, or use the "Send Job to Mini Cutter" to send the currently-open 2D design to the Mini for cutting, or drawing.  You can also use the "Mini Cutter Settings" submenu to set the rate at which the tool moves when cutting or drawing (maximum recommended is 100).  Note; the "Use Path Planner" checkbox, if set, tells LaserCut to optimize the order in which it cuts, or draw lines to make inner cuts in a shape before making outer cuts.  This option is more useful for laser cutters, but I recommend you leave it checked unless you are having some problems with how the Mini is cutting, or draw, in which case you can try unchecking it.
 
 Unlike Provo Craft's proprietary software, LaserCut is open source and also able to read vector images from DXF Files (2D only), SVG Files and LaserCut's .lzr file format.  In addition, it can export 2D designs, including those you create using LaseCut's design tools, to SVG files, DXF files, EPS and PDF files.
 
 ## Make Your Own Cutting Mats for the Mini
 
 Having discontinued support for the Mini, Provo Craft also stopped selling cutting mats that will fit into the Mini's feeder mechanism and the mats used by the Mini are hard to obtain and also different in size from those used by other cutters made by Provo Craft.  However, after some research, I was able to track down a simple way to make my own cutting maps from inexpensive and easily available (at the time this was written) items.  Here's what you'll need:
 
  1. [Chop Chop 9.5" x 14" The Original Flexible Cutting Mat](https://www.amazon.com/gp/product/B00TZ0F556) (you get 6 mats for $6!)
  2. [Krylon K07020007 10.25-Ounce Easy Tack Repositionable Adhesive Spray](https://www.amazon.com/gp/product/B000A8AYO4/)
  3. [1" wide Blue Painters Tape](https://www.amazon.com/gp/product/B01MYP8R5B/)
  4. [Grafix Clear .003 Dura-Lar Film, 9-Inch by 12-Inch, 25 Sheets](https://www.amazon.com/gp/product/B002544NHA/) (used to cover and protect the sticky area of the mats)

Start by covering all the edges of the cutting mat with Blue Painter's Tape.  The tape should be used to mask off the 1/2 inch area on the sides of the mat where the feed rollers engage as well as the 1 inch areas at the top and bottom of the mat, which are outside the area where the Mini typically cuts, or draws.  Wrap any excess tape around to the backside of the mat.  Once, the edges are masked, place the masked mat onto a sheet of newspaper and carefully spray the unmask area of the cutting mat with the Krylon spray.  Then, after the Krylon spray has dried ([3 hours according to the manufacturer](https://www.krylon.com/products/easytack-repositionable-adhesive/)), test the stickiness of the surface by applying some the material you intend to use in the Mini.  Use additional Krylon spray, if needed, to increase the tackiness.  Remove the tape when you're satisfied with the stickiness and try out your newly-made cutting mat in the Mini.  When not in use, cover and protect the tacky surface of the mat using a sheet of the Grafix Dura-Lar film, or some other, similar material.

In the future, as the mat becomes less sticky with wear and use, you may be able to remove the old coating of Krylon spray using %100 mineral oil and then, after washing the mat with soap and water to remove the mineral oil and then drying, you should be able to reapply a new coating coating

## Replacement Blade Holder for the Mini

My used MIni did not come with a blade holder, but I was able to purchase a 3rd party-made replacement on eBay for $7 (with free shipping.)  The Mini uses the same blade holder as other Provo Craft "legacy" machine, such as the Personal/V1 (CRV001), Expression (CREX001), Expression 2 (CREXOO2), Imagine (CRIM0001) and Create (CRV2001) so, until all these machine are discontinued it should be possible to get replacement parts.

