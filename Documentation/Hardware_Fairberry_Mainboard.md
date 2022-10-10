# Bill of Materials

- Blackberry Q10 replacement keyboard
- Assembled mainboard from JLCPCB
- Hirose BM14B(0.8)-24DS-0.4V(53)
- Solderable USB 2.0 OTG connector (can be USB C, Micro USB or even Lightning)
- Thin wires to connect the USB board to the PCB (The thinner the better)
- 6 Pin headers
- USBASP Programmer
- Jumper wires

# Ordering from JLCPCB

## Downloads

First go to the release page (https://github.com/Dakkaron/Fairberry/releases) and locate the newest release.

Download gerber-*.zip, BOM.xlsx and CPL.xlsx.

## PCB page

Now head to the JLCPCB PCB ordering page: https://cart.jlcpcb.com/quote?orderType=1&stencilLayer=2&stencilWidth=100&stencilLength=100&stencilCounts=5

Upload the gerber-*.zip file. You can leave everything in the main box on default.

Scroll down until you find "PCB Assembly" and activate that.

Depending on how many parts you want to order, select 2 or 5 under "PCB Assembly". Ordering more parts will be cheaper per part. Please note, you will receive 5 boards regardless, but only the selected amount will be populated (will have components soldered to it).

Now press "Next".

## BOM and CPL

On the following page, upload the BOM.xmlx and the CPL.xmlx. On the dropdown on the bottom of the page select "Office Appliance and Accessories" -> "Keyboard".

Press "Next" again.

## Component list

You should now see a list of components. Verify that no errors (e.g. "Component out of stock") are displayed and all parts are selected.

Press "Next" again.

## Component placement

The next page shows a rendering of the component placement. Verify (multiple times) that the image looks exactly as the image below. Especially all the pink markings need to be exactly the same.

![Component placement image](https://github.com/Dakkaron/Fairberry/blob/main/Images/CPL_v0.2.0.png)

If one of the parts is rotated wrong (indicated by the pink markings being in the wrong location), select the component and use the "All Rotate Right/Left" buttons at the top to fix it.

If you are really sure it looks the same, press "Save to cart" and procede with ordering the PCB.


# Remaining Assembly

Sadly, JLCPCB doesn't supply the Hirose connector. If you want to, you can submit a parts request with them to maybe sway their mind.

This is a rather difficult part to solder, since the pins are so tiny. If you don't have much experience, watch a few videos on how to solder QFN44 chips (they have the same pin dimensions) and practice on some scrap parts.

Once this is done, you can solder the pin headers like in the picture below. These pin headers are only required for initial flashing, and can be desoldered afterwards, as they won't be needed anymore.

![Placement of programming pin headers](https://github.com/Dakkaron/Fairberry/blob/main/Images/programmer_pin_headers.jpeg)

Now take your USBASP and some jumper wires and connect the headers to the fitting pins on the programmer, following the images below. The pinout of the USBASP is shown here as viewed into the connector, with no cable plugged in.

Make sure you are using pinout for the right mainboard version.

## USBASP
![USBASP pinout](https://github.com/Dakkaron/Fairberry/blob/main/Images/USBASP_pinout.png)

## v0.1.1
![Fairberry v0.1.1 pinout](https://github.com/Dakkaron/Fairberry/blob/main/Images/Pinout_v0.1.1.png)

## v0.2.0
![Fairberry v0.2.0 pinout](https://github.com/Dakkaron/Fairberry/blob/main/Images/Pinout_v0.2.0.png)

Now solder the USB connector to the USB port on the Fairberry mainboard. Take care to get the pinout right and not cause any shorts.

![Fairberry v0.1.1 with USB connector](https://github.com/Dakkaron/Fairberry/blob/main/Images/usb_plug_soldered.jpeg)

# Flashing the firmware

## Flashing the bootloader

This step only needs to be done once. After you flashed the bootloader, you can update the firmware over the USB connector.

Download and install the [Arduino IDE](https://www.arduino.cc/en/software).

Open the Arduino IDE.

Connect the USBASP with the connected Fairberry Mainboard to a USB port of the computer.

WARNING: DO NOT PLUG THE USB CONNECTOR OF THE FAIRBERRY INTO THE PC, WHILE THE USBASP IS CONNECTED!

In the Arduino IDE, select "Tools" -> "Board" -> "Arduino AVR Boards" -> "Arduino Leonardo". Then select "Tools" -> "Burn Bootloader".

This will take a while. Wait until it finishes. If there are errors, check your solder joints and the wires connecting the Fairberry to the USBASP.

Once this is done, disconnect the Fairberry from the USBASP. You can now desolder the header pins.

## Flashing the firmware

Head to the [releases page](https://github.com/Dakkaron/Fairberry/releases), download and unzip the file BBQ10_Arduino_sources.zip.

Open the BBQ10.ino in the Arduino IDE.

In the Arduino IDE select "Tools" -> "Manage Libraries".

In the Library Manager, check that the following libraries are installed (and install them if they aren't):

- Keyboard, by Arduino, version 1.0.4 or newer
- LowPower_LowPowerLab, by LowPowerLab, version 2.2.0 or newer.

Now close the Library Manager.

Open the file configuration.h in the Arduino IDE.

Find this line

    #define BOARD_TYPE FAIRBERRY_V0_2_0

and adjust the board type to the version that you are using.

Make sure the USBASP is disconnected from the PC and the Fairberry Mainboard.

Connect the Fairberry Mainboard's USB connector to the PC.

Select "Sketch" -> "Upload" and wait for the upload to finish.

If you get an error, verify your soldering.

# Testing

Now you are basically done. To test the mainboard, connect the Blackberry Q10 keyboard to the connector and plug the whole thing into the USB/Lightning port of your phone. Open some app where you can type and check if every key works as expected.

## Troubleshooting

If some keys work but others don't, some of the pins on the Hirose connector aren't soldered correctly. To figure out which ones, check out this [Column to row key matrix](https://github.com/arturo182/BBQ10KBD#columnrow-to-key-matrix).  With this you can figure out, which rows/colums have bad contacts.

Now look at this [breakout adapter](https://github.com/arturo182/bbq10kbd_breakout) and follow the traces from the defective row/column pin to the pin on the connector to figure out, which pin has a bad contact.

If you know which pin is at fault, try to carefully solder it on with a fine-tipped soldering iron.
