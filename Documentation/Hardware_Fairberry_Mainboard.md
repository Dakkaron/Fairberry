# Bill of Materials

- Blackberry Q10 replacement keyboard
- Assembled mainboard from JLCPCB
- Solderable USB 2.0 OTG connector (can be USB C, Micro USB or even Lightning)
- Thin wires to connect the USB board to the PCB (The thinner the better)
- 6 Pin headers
- USBASP Programmer
- Jumper wires

# Ordering from JLCPCB

## Downloads

First go to the release page (https://github.com/Dakkaron/Fairberry/releases) and locate the newest release.

Download gerber-*.zip, BOM.xlsx and CPL.xlsx.

## Order Hirose Keyboard Connector

JLCPCB does not usually stock the keyboard connector (Hirose BM14B(0.8)-24DS-0.4V(53)). To get them to include this part in the assembly, you first need to order the part in their part sourcing programme. To do so:

- Head to the part ordering page: https://jlcpcb.com/parts
- Log in to your JLCPCB account (create one if you don't have one)
- At the part ordering page, search for this part: HRS(Hirose) BM14B(0.8)-24DS-0.4V(53)
- There should be one result. Select it, choose an amount (2 or multiples of 5) and then press "Add to My Part Lib" and follow the order process
- Sourcing the components will take a few weeks
- Once they sourced the parts, they will refund you the difference between what you paid and what JLCPCB paid to the part seller, so it will likely be a bit cheaper than what you paid up front

## Order PCB

- Head to the JLCPCB PCB ordering page: https://cart.jlcpcb.com/quote?orderType=1&stencilLayer=2&stencilWidth=100&stencilLength=100&stencilCounts=5
- Upload the gerber-*.zip file. You can leave everything in the main box on default.
- Scroll down until you find "PCB Assembly" and activate that.
- Depending on how many parts you want to order, select 2 or 5 under "PCB Assembly". Ordering more parts will be cheaper per part. Please note, you will receive 5 boards regardless, but only the selected amount will be populated (will have components soldered to it).
- Press "Next".

## BOM and CPL

On the following page, upload the BOM.xmlx and the CPL.xmlx. On the dropdown on the bottom of the page select "Office Appliance and Accessories" -> "Keyboard".

Press "Next" again.

## Component list

You should now see a list of components. Verify that no errors (e.g. "Component out of stock") are displayed and all parts are selected. You should be able to take the Hirose connector from your part library.

Press "Next" again.

## Component placement

The next page shows a rendering of the component placement. Verify (multiple times) that the image looks exactly as the image below. Especially all the pink markings need to be exactly the same.

![Component placement image](https://github.com/Dakkaron/Fairberry/blob/main/Images/CPL_v0.3.0.png)

If one of the parts is rotated wrong (indicated by the pink markings being in the wrong location), select the component and use the "All Rotate Right/Left" buttons at the top to fix it.

If you are really sure it looks the same, press "Save to cart" and procede with ordering the PCB.


# Remaining Assembly

Solder the programming headers like in the picture below. These pin headers are only required for initial flashing, and can be desoldered afterwards, as they won't be needed anymore.

![Placement of programming pin headers](https://github.com/Dakkaron/Fairberry/blob/main/Images/programmer_pin_headers.jpeg)

Now take your USBASP and some jumper wires and connect the headers to the fitting pins on the programmer, following the images below. The pinout of the USBASP is shown here as viewed into the connector, with no cable plugged in.

Make sure you are using pinout for the right mainboard version.

## USBASP
![USBASP pinout](https://github.com/Dakkaron/Fairberry/blob/main/Images/USBASP_pinout.png)

## v0.1.1
![]()
<img alt="Fairberry v0.1.1 pinout" src="https://github.com/Dakkaron/Fairberry/blob/main/Images/Pinout_v0.1.1.png" style="width:30%">

## v0.2.0
<img alt="Fairberry v0.2.0 pinout" src="https://github.com/Dakkaron/Fairberry/blob/main/Images/Pinout_v0.2.0.png" style="width:30%">

## v0.3.0
<img alt="Fairberry v0.3.0 pinout" src="https://github.com/Dakkaron/Fairberry/blob/main/Images/Pinout_v0.3.0.png" style="width:30%">

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

    #define BOARD_TYPE FAIRBERRY_V0_3_0

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

## Generating the 3D-print file for the case

### Prerequisites
- Install [OpenSCAD](https://openscad.org/)
- Download and extract the Source Code zip file from [the newest release](https://github.com/Dakkaron/Fairberry/releases)
- Open `Fairberry-x.x.x/Case/Generator/Fairberry.scad` in OpenSCAD

### Customizing
- In the `Fairberry.scad` script, set the `SCREEN_PROTECTOR_HEIGHT` to the thickness of your screen protector. If you don't use one, set it to `0`.
- Set the `EXTRUSION_WIDTH` to the extrusion width of your 3D printer. For 0.4mm nozzles, it's usually 0.48. If you use a resin printer, leave it at 0.4.
- If you are using a Fairphone 4 or a Samsung Galaxy A54, comment out the line `include <presets/Custom.scad>` by placing a double slash (`//`) at the beginning of the line, and uncomment the line fitting your phone. You can skip the rest of the customization.
- If you are using a different phone, open up `Fairberry-x.x.x/Case/Generator/presets/Custom.scad` in OpenSCAD and customize the values in there to your liking. To preview the changes, save your changes in the `Custom.scad` and then open up `Fairberry.scad` in OpenSCAD and press F5 to generate a preview.
- When using `Custom.scad`, it is recommended to find a dummy STL of your phone. [Yeggi.com](https://www.yeggi.com/) and [Grabcad](https://grabcad.com/) are good sources for dummy phone models.

### Generating the file
- Once everything is customized to your liking, open `Fairberry.scad` and press F5 to generate a preview.
- Press F6 to render the file (this may take multiple minutes depending on your hardware).
- Press F7 and save the resulting STL file as `Fairberry.stl`.

## Printing the case

You need to print two files: The `Fairberry.stl` you generated in the last step and the `Fairberry-x.x.x/Case/Generator/USB_Clamp.stl`.

If you don't own a 3D printer, [JLCPCB](https://jlc3dp.com/3d-printing-quote) is a good place to start.

## Assembly

First secure the flex PCB of the keyboard with a bit of kepton tape. This is done to reduce the risk of ripping the flex PCB at the point where it enters the keyboard.

![Kepton on flex PCB](https://github.com/Dakkaron/Fairberry/blob/main/Images/kepton_on_flexpcb.jpg)

Next put a small roll of Blu-Tack/Patafix onto the flex PCB and CAREFULLY bend it, like in the picture below. Warning, bending needs to be done carefully, otherwise it can break. Don't bend it more often than necessary. The roll of Blu-Tack/Patafix is there to cushion the bend a bit, so that it doesn't have as much strain on it.

![Folding flex PCB](https://github.com/Dakkaron/Fairberry/blob/main/Images/folding_flexpcb.jpg)

Optionally you can wrap the mainboard in kepton tape. This reduces the risk for shorts and keeps it clean from Blu-Tack/Patafix. Make sure to keep the keyboard connector free of kepton.

![Mainboard wrapped in kepton](https://github.com/Dakkaron/Fairberry/blob/main/Images/pcb_in_kepton.jpg)

Put some thin blobs of Blu-Tack/Patafix onto the backside of the mainboard and plug the keyboard's flex PCB into the mainboard.

![Patafix on mainboard](https://github.com/Dakkaron/Fairberry/blob/main/Images/patafix_on_pcb.jpg)

Carefully bend the Flex PCB into an S-shape under the mainboard and very carefully press the mainboard onto the keyboard, so that the Blu-Tack/Patafix can stick to the keyboard. Optionally, you can now also put a strip of kepton over the keyboard connector, to secure it to the mainboard.

![Mainboard on keyboard](https://github.com/Dakkaron/Fairberry/blob/main/Images/mainboard_on_keyboard.jpg)

Put the M3 screws through the hole in the case and place the USB connector into the fitting hole.

![Preparing USB connector](https://github.com/Dakkaron/Fairberry/blob/main/Images/usb_connector_prepared.jpg)

Put the M3 nuts into the holes in the USB clamp and screw it down. To help aligining the USB connector, place your phone into the attachment and plug the USB connector into the phone, before screwing it down. This makes sure that the USB corrector is aligned correctly.
Also put a thin roll of Blu-Tack/Patafix on the top part of the area where the keyboard goes.

![Preparing USB connector](https://github.com/Dakkaron/Fairberry/blob/main/Images/usb_screwed_down.jpg)

Lastly, hook the bottom lip of the keyboard into the fitting hole in the case and carefully slide it in at an angle from the top. It might take a bit of careful force to put it in.

![Fully assembled attachment](https://github.com/Dakkaron/Fairberry/blob/main/Images/assembly_finished.jpg)
