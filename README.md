# Fairberry

The goal of this project is to add a BlackBerry Q10 keyboard to non-BlackBerry phones. While this can easily be adapted to suit any other phone with an USB port at the bottom, I will focus on releasing files for the Fairphone 4, since that is what I have. Feel free to adjust it to your phone and share your results in the discussion area.

The Fairberry keyboard is detachable and can be used without any modifications on the phone. Since it connects over USB, there is no need for separate batteries or glitchy Bluetooth connections.

## What can you expect?

![Frontal shot of the keyboard attached to a Fairphone 4](https://github.com/Dakkaron/Fairberry/raw/main/Images/keyboard_attached_top.jpeg)
![Side shot of the keyboard attached to a Fairphone 4](https://github.com/Dakkaron/Fairberry/raw/main/Images/keyboard_attached_side.jpeg)
![Bottom shot of the keyboard attached to a Fairphone 4](https://github.com/Dakkaron/Fairberry/raw/main/Images/keyboard_attached_underside.jpeg)
![Looking into the phone socket of the detacked keyboard](https://github.com/Dakkaron/Fairberry/raw/main/Images/keyboard_detached_top.jpeg)

## How is it working?

![Underside of the electronics](https://github.com/Dakkaron/Fairberry/raw/main/Images/electronics_underside.jpeg)
![Other angle of the underside of the electronics](https://github.com/Dakkaron/Fairberry/raw/main/Images/electronics_underside_top.jpeg)

There are five parts to the Fairberry keyboard:

- A BlackBerry Q10 replacement keyboard from Aliexpress (~€10-15 plus shipping)
- The BlackBerry Q10 Keyboard Breakout Board from here: https://github.com/arturo182/bbq10kbd_breakout
- An Arduino Pro Micro (Micro, not Mini! This is very important since the Pro Mini does not have USB interface that can be used to emulate a keyboard!)
- An USB C or Micro USB OTG breakout board or cable to connect to the phone
- The 3D printed case

> :warning: **Please read (and understand) the whole page before buying/building it!**
> :warning: **Messing this up can result in damage to your phone, your PC and/or the parts used in this build. If you are new to soldering, ask someone for help! I am not responsible for any damage that you do to your stuff! You are doing this at your own risk!**

### The BlackBerry Q10 Keyboard

This is just a regular keyboard for the BBQ10. You can either buy a replacement one from eBay or Aliexpress, or scavenge one off a BlackBerry Q10. Better get multiple ones, if shipping is slow. The flex PCB needs to be bent and can break in this process.

I decided to use the Q10's keyboard (and not, for example, the Key1/Key2 keyboard) since the Q10 keyboard is documented really nicely, and there are ready-made adapter designs, that I could use without having to design my own PCB.

I took the US Keyboard layout, since it was the easiest to get. If you get another keyboard layout you will need to adjust the keyboard mapping in the Arduino sketch.

### The BlackBerry Q10 Breakout Board

You can get the breakout board design files from here: https://github.com/arturo182/bbq10kbd_breakout

To get the board printed, you need to download the bbq10kbd_breakout repository files ("Code" -> "Download ZIP"). Next, put all files in the folder named "gerber" in a zip file and upload it here: https://jlcpcb.com/

The minimum order quantity at JLCPCB is 5. It is not a mistake to order more, this part is really tricky to solder. If you haven't soldered small SMD components before, you will need a few tries.

You also need to buy some Hirose BM14B(0.8)-24DS-0.4V(53) connectors. They are a bit hard to get. One place to get them from is https://www.mouser.at/ProductDetail/Hirose-Connector/BM14B08-24DS-04V53?qs=QRVci0nWsCaR3TeKMbLD%252Bw%3D%3D

Also here, get a few of them. You are going to need them.

Once you have those parts, the connector needs to be soldered to the board.

If you don't feel comfortable soldering something that tiny, go to your local phone/laptop repair shop. They usually have the equipment and skill to solder something like that for a few Euros.

If you've done that, that was the trickiest part.

Make sure you verify with a multimeter that all the conenctions are working and there are no shorts.

### The Arduino Pro Micro

Make sure you get an Arduino Pro Micro, and not the Pro Mini. The Pro Mini uses a different chip that doesn't natively support USB, and it will not work. To work, your Pro Micro should have an AtMega32u4 chip on it. It doesn't matter too much what kind of USB connector is on there, since we are going to remove that one anyways.

First, flash the BBQ10.ino sketch to your Arduino. For that, download the BBQ10 folder in this repository, download the Arduino IDE, and open the BBQ10.ino in the Arduino IDE.

Next, select "Tools" -> "Board" -> "Arduino Leonardo".
Now select "Tools" -> "Port" and note down all ports listed there (if there are any)
Then connect the Arduino to the PC using an USB cable, and recheck "Tools" -> "Port" for the new port that should have appeared. Select that port.
Now you can flash the sketch to the Arduino using "Sketch" -> "Upload".

You can now desolder the USB socket from the Arduino.

### The USB connector

Depending on your phone, get an USB C or Micro USB connector. You can either use OTG Breakout Boards (they give really good results, since they are very small) or USB 2.0 OTG cables (they are easier to find and easier to work with). What you choose is up to you.

If you got a cable, cut it about 5 cm from the connector. Strip about 2 cm of the outer plastic and shielding. Cut off any bare wires, so that there are only four cables left over. These should be red (5V), black (GND), green (Data-) and white (Data+). Strip the ends of the four cables and spread them apart. Make sure they don't touch. Now plug them into your phone and, using a multimeter in voltage mode, touch the red and black wires with the respective colored leads on the multimeter. Check that the multimeter displays 5V. If it doesn't, throw away the cable and get a new one. The reason is, that there are some extremely cheaply made USB C cables with wrong wireing. One of these cables might fry your keyboard and/or your phone. Make sure, you detach the cable from the phone when you are done. Shorting these cables while connected to your phone could also fry your phone.

### The Case

The cases are still work in progress.

The cases are plug-and-play, so you can easily replace the case at a later date, without having to change anything about the electronics.

Currently, there are two options: A bottom-only case (which is what is shown in the pictures above) and a full-body multimaterial case. For the multimaterial case you need a 3D printer capable of handling two different materials at the same time (e.g. by having two extruders). Also, the printer needs to be able to handle flexible materials.

The files for the cases are in the Cases-folder of this repository.

Print files with _PLA in the name in any hard plastic of your choice (PLA, PETG, ABS, ...). Any will work.

Print files with _TPU in the name in any flexible plastic of your choice (TPU, TPE). A bit harder might be better, but since I have only used one type of flexible filament (FlexiSmart), your milage may vary.

## Assembling all the parts

Assembly is a bit tricky. You will need either some double-sided tape or (what I used) some putty-like adhesive (e.g. Blu Tack or Patafix).


First, take the connector and the Arduino. Put them next to each other in the orientation shown on the following image:
![Underside of the electronics](https://github.com/Dakkaron/Fairberry/raw/main/Images/electronics_underside.jpeg)

Using some thin enameled wire and solder, connect the pins like in the following table:

| Adapter | Arduino |
| ------- | ------- |
| Row 1   | 9       |
| Row 2   | 8       |
| Row 3   | 7       |
| Row 4   | 6       |
| Row 5   | 5       |
| Row 6   | 4       |
| Row 7   | 3       |
| Col 1   | A1      |
| Col 2   | A0      |
| Col 3   | 15      |
| Col 4   | 14      |
| Col 5   | 16      |
| GND     | GND (I used the one next to Pin 2) |
| VCC     | 10 (Used for the backlight) |

Do not connect the MIC or the unlabelled connector on the bottom right of the adapter.

The next step is to solder the USB connector to the Pro Micro. In the image you can see a small resistor at D+. You will not need that one. My Arduino had a defective resistor, and this is just overriding it. So, just ignore that resistor.
The following image shows where each of the USB connector's leads need to be soldered to:
![Underside of the electronics](https://github.com/Dakkaron/Fairberry/raw/main/Images/electronics_underside.jpeg)

Now take the BlackBerry Q10 keyboard, and put it face down. Check the picture below for the finished assembly positions:

Next, take some Blu Tack/Pattafix/... and put a small bit on the keyboard, where the Arduino will come to rest. You will now have to bend the flex PCB in an S shape. Make sure you don't fold the PCB, since that might break it. Put some 1-2mm thick ropes of Blu Tack/Pattafix in the bends to make sure they don't fold. Connect the connector to your adapter. Make the S bend and stick the electronics down with a thin layer of Blu Tack/Pattafix.

The keyboard itself is now ready. You can connect it to a smartphone or keyboard and see if all keys work. You will need to set the keyboard layout on your device to standard (no layouts activated) if you get incorrect output. This is necessary, because Android 11 has a bug that distorts all other layouts if an accessibility service is activated.

If everything is working, it's now time to attach the keyboard to your phone.

Put a thin roll of Blu Tack/Pattafix at the top rim of the keyboard. This is used to stick the keyboard to the case. Slide the keyboard from the top into the recess at the bottom of the keyboard, and fold it down.

Your keyboard is now ready to be used!

## Android adjustments:

If the keyboard overlaps too much with your screen, install adb on your PC and enable Developer Options on your phone. Connect the phone to the PC and open a shell window (under Windows cmd, under Linux/MacOS use bash or whatever you have) and execute the following command:

```
adb shell wm size
```

This shows the current resolution of the smartphone. Now reduce the second number by doing this:

```
adb shell wm size 1080x2200
```

You will have to figure out what resolution you want to use, so just try out which values fit for you.
To reset it to default, execute this command:

```
adb shell wm size reset
```
