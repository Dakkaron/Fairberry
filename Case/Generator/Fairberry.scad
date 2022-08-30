include <include/roundedcube.scad>

//Phone dimensions in mm
PHONE_WIDTH=75.5;
PHONE_THICKNESS=12.0;
PHONE_KEYBOARD_OVERLAP=11.5; // Determines how much the keyboard will overlap the bottom of the phone

// Leave empty ("") to generate the phone shape using this script
// PHONE_DUMMY_FILE="";
PHONE_DUMMY_FILE="import/FP4_Dummy_Screenprotector.stl";

// Corner radius of the phone. Only relevant if PHONE_DUMMY_FILE==""
PHONE_ROUNDING_RADIUS=6;

EXTRUSION_WIDTH=0.48; // Extrusion width of your 3D printer
PERIMETERS=3; // How many perimeters should the top part of the case be thick?

// How many perimeters should the case be thick below the phone?
CASE_BOTTOM_PERIMETERS=2;

CASE_SLEEVE_LENGTH=30; // Length of the case extension towards the top of the phone
CASE_ROUNDING_RADIUS=5; // Corner radius of the case.
CASE_TOP_ROUNDING_OFFSET=1; // Increasing this number makes the top less rounded

KEYBOARD_PHONE_GAP_PERIMETERS=1; // Thickness of the separator between keyboard and phone screen

_ELECTRONICS_CUTAWAY_WIDTH=60.0;
_ELECTRONICS_CUTAWAY_THICKNESS=PHONE_THICKNESS*.8;
_ELECTRONICS_CUTAWAY_LENGTH=20;
_KEYBOARD_THICKNESS=3.9;
_CASE_BOTTOM_LENGTH=38;
_CASE_KEYBOARD_Y_OVERLAP=EXTRUSION_WIDTH*2;

_USB_PORT_CUTAWAY_WIDTH=10;
_USB_PORT_CUTAWAY_HEIGHT=3.5;
_USB_PORT_CUTAWAY_LENGTH=20;
_USB_PORT_ROUNDING_RADIUS=2;

//Case cannot get smaller than 70mm, due to keyboard width
CASE_WIDTH=max(PHONE_WIDTH, 70.0) + EXTRUSION_WIDTH*PERIMETERS*2;
CASE_THICKNESS=PHONE_THICKNESS + EXTRUSION_WIDTH*CASE_BOTTOM_PERIMETERS + _KEYBOARD_THICKNESS + KEYBOARD_PHONE_GAP_PERIMETERS*EXTRUSION_WIDTH;

module keyboardDummy() {
    color("darkgrey")
    translate([0,0,PHONE_THICKNESS/2 + KEYBOARD_PHONE_GAP_PERIMETERS*EXTRUSION_WIDTH])
        import("import/BBQ10KBD.stl", convexity=5);
}

module phoneDummy() {
    if (PHONE_DUMMY_FILE==""){
        color("purple")
        translate([0,-PHONE_KEYBOARD_OVERLAP,0])
        union() {
            translate([-PHONE_WIDTH/2, 0, -PHONE_THICKNESS/2])
            roundedcube(size=[PHONE_WIDTH, 50+CASE_SLEEVE_LENGTH, PHONE_THICKNESS], center=false, radius=PHONE_ROUNDING_RADIUS, apply_to=["all"]);
            translate([-_USB_PORT_CUTAWAY_WIDTH/2, -_USB_PORT_CUTAWAY_LENGTH/2, -_USB_PORT_CUTAWAY_HEIGHT/2])
    roundedcube(size=[_USB_PORT_CUTAWAY_WIDTH, _USB_PORT_CUTAWAY_LENGTH, _USB_PORT_CUTAWAY_HEIGHT], center=false, radius=_USB_PORT_ROUNDING_RADIUS, apply_to=["y"]);
        }
    } else {
        color("purple")
        translate([0,-PHONE_KEYBOARD_OVERLAP,0])
        import(PHONE_DUMMY_FILE, convexity=5);
    }
}

module phoneRoundingBox() {
    translate([-CASE_WIDTH/2,_CASE_KEYBOARD_Y_OVERLAP-_CASE_BOTTOM_LENGTH,-PHONE_THICKNESS/2 - EXTRUSION_WIDTH*CASE_BOTTOM_PERIMETERS])
    roundedcube(size=[CASE_WIDTH,_CASE_BOTTOM_LENGTH+100, CASE_THICKNESS + CASE_TOP_ROUNDING_OFFSET], center=false, radius=CASE_ROUNDING_RADIUS, apply_to=["all"]);
}

module case() {
    translate([-CASE_WIDTH/2, _CASE_KEYBOARD_Y_OVERLAP-_CASE_BOTTOM_LENGTH, -PHONE_THICKNESS/2 - EXTRUSION_WIDTH*CASE_BOTTOM_PERIMETERS])
    cube(size=[CASE_WIDTH, _CASE_BOTTOM_LENGTH, CASE_THICKNESS], center=false);
}

module caseSleeve() {
    translate([-CASE_WIDTH/2, 0, -PHONE_THICKNESS/2-0.05 - EXTRUSION_WIDTH*CASE_BOTTOM_PERIMETERS])
    roundedcube(size=[CASE_WIDTH, CASE_SLEEVE_LENGTH, PHONE_THICKNESS+EXTRUSION_WIDTH*CASE_BOTTOM_PERIMETERS-0.05], center=false, radius=CASE_ROUNDING_RADIUS, apply_to=["y"]);
}

module electronicsCutaway() {
    color("pink")
    translate([-_ELECTRONICS_CUTAWAY_WIDTH/2, -12-_ELECTRONICS_CUTAWAY_LENGTH, KEYBOARD_PHONE_GAP_PERIMETERS*EXTRUSION_WIDTH+0.1-PHONE_THICKNESS*.3])
    cube(size=[_ELECTRONICS_CUTAWAY_WIDTH, _ELECTRONICS_CUTAWAY_LENGTH, _ELECTRONICS_CUTAWAY_THICKNESS], center=false);
}

module usbAnvil() {
    translate([0,-PHONE_KEYBOARD_OVERLAP,0])
    import("import/USB_Anvil.stl");
}

module countersunkM3() {
    translate([0,0,12])
    union() {
        cylinder(20,d1=3,d2=3,center=true);
        translate([0,0,-10])
        cylinder(2,d1=6.5,d2=3,center=true);
    }
}

module usbScrewHoles() {
    union() {
        translate([-10,-PHONE_KEYBOARD_OVERLAP-4.25,4-PHONE_THICKNESS])
        countersunkM3();
        translate([10,-PHONE_KEYBOARD_OVERLAP-4.25,4-PHONE_THICKNESS])
        countersunkM3();
    }
}

translate([0,0,_CASE_BOTTOM_LENGTH])
rotate([90,0,180])
intersection() {
    union() {
        difference() {
            union() {
                case();
                caseSleeve();
            }
            keyboardDummy();
            phoneDummy();
            electronicsCutaway();
            usbScrewHoles();
        }
        usbAnvil();
    }
    phoneRoundingBox();
}