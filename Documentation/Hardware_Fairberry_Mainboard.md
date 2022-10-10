# Bill of Materials

- Blackberry Q10 replacement keyboard
- Assembled mainboard from JLCPCB
- Hirose BM14B(0.8)-24DS-0.4V(53)
- Solderable USB 2.0 OTG connector (can be USB C, Micro USB or even Lightning)
- Thin wires to connect the USB board to the PCB (The thinner the better)

# Ordering from JLCPCB

First go to the release page (https://github.com/Dakkaron/Fairberry/releases) and locate the newest release.

Download gerber-*.zip, BOM.xlsx and CPL.xlsx.

Now head to the JLCPCB PCB ordering page: https://cart.jlcpcb.com/quote?orderType=1&stencilLayer=2&stencilWidth=100&stencilLength=100&stencilCounts=5

Upload the gerber-*.zip file. You can leave everything in the main box on default.

Scroll down until you find "PCB Assembly" and activate that.

Depending on how many parts you want to order, select 2 or 5 under "PCB Assembly". Ordering more parts will be cheaper per part. Please note, you will receive 5 boards regardless, but only the selected amount will be populated (will have components soldered to it).

Now press "Next".

On the following page, upload the BOM.xmlx and the CPL.xmlx. On the dropdown on the bottom of the page select "Office Appliance and Accessories" -> "Keyboard".

Press "Next" again.

You should now see a list of components. Verify that no errors (e.g. "Component out of stock") are displayed and all parts are selected.

Press "Next" again.

The next page shows a rendering of the component placement. Verify (multiple times) that the image looks exactly as the image below. Especially all the pink markings need to be exactly the same.

If one of the parts is rotated wrong (indicated by the pink markings being in the wrong location), select the component and use the "All Rotate Right/Left" buttons at the top to fix it.

If you are really sure it looks the same, press "Save to cart" and procede with ordering the PCB.
