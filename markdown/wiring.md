## Wiring

<p align="center"><a href="https://github.com/wholder/TeensyCNC2/blob/master/images/teensypads.jpg"><img src="https://github.com/wholder/TeensyCNC2/blob/master/images/teensypads.jpg" width="95%" height=95%"></a><br>Click for larger view</p>

For each pad shown in the picture above, cut a length of 30 gauge wire wrap wire long enough to reach from the pad on the PCB to the indicated pad on the Teensy (note: be careful to connect to the correct pad on the Teensy as some of the pads, such as those for pins 21, 22 amd 23 are labelled at an angle on the Teensy.)

Next, strip just enough insulation from the end of each wire to make a good connection to the appropriate pads.  Since you've already applied 60/40 solder to these pads, you should be able to hold the tin-plated wire to the pad and touch it with the soldering iron to reflow the solder to make a good connection.  Then, bend the wire, as needed to route it to the conecting pad on the Teensy.  I recommend that you also first preapply 60/40 solder to the needed pads on the Teensy so that you make the connection on the Teensy side by holding the wire to the pad while you reapply heat with the soldering iron.

Repeat the above steps for all of the needed connections.  I reccommend first making all the connections from the 14 Pin PIN and then making the connections from the 28 pin PIC and from resistor R7, as listed below:

 - Connect **D8** on the PCB to **pin D8** on the Teensy
 - Connect **D7** on the PCB to **pin D7** on the Teensy
 - Connect **D6** on the PCB to **pin D6** on the Teensy
 - Connect **D11** on the PCB to **pin D11** on the Teensy
 - Connect **D12** on the PCB to **pin D12** on the Teensy
 - Connect **D16** on the PCB to **pin D16** on the Teensy
 - Connect **D17** on the PCB to **pin D17** on the Teensy
 - Connect **D14** on the PCB to **pin D14** on the Teensy
 - Connect **D13** on the PCB to **pin D13** on the Teensy
 - Connect **D5** on the PCB to **pin D5** on the Teensy
 - Connect **Gnd** on the PCB to the **Gnd pin closest to the USB connector** on the Teensy
 - Connect **USB+** on the PCB to the **D+ pad** on the Teensy
 - Connect **USB-** on the PCB to the **D- pad on** the Teensy
 - Connect **D21** on the PCB to **pin D21** on the Teensy
 - Connect **D20** on the PCB to **pin D20** on the Teensy
 - Connect **D22** on the PCB to **pin D22** on the Teensy
 - Connect the **pad on the left side of R7** to the **VUSB pad** on the Teensy
 
 When completed, your PCB connections should look like this:
 
 <p align="center"><a href="https://github.com/wholder/TeensyCNC2/blob/master/images/teensymod.jpg"><img src="https://github.com/wholder/TeensyCNC2/blob/master/images/teensymod.jpg" width="95%" height=95%"></a><br>Click for larger view</p>
 

