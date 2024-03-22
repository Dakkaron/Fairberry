// Fairberry Keyboard Case Generator © 2024 by Dakkaron is licensed under Attribution-NonCommercial-ShareAlike 4.0 International.
// To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/
// Version 2

SCREEN_PROTECTOR_HEIGHT=0.6; // Thickness of the screen protector
EXTRUSION_WIDTH=0.4; // Extrusion width of your 3D printer

// Uncomment the type of phone you use. If your phone doesn't appear in this list, leave "include <presets/Custom.scad>" and edit the file at presets/Custom.scad to fit your phone.
include <presets/Custom.scad>
//include <presets/SamsungGalaxyA54.scad>
//include <presets/Fairphone4.scad>


_KEYBOARD_CATCH_DEPTH=1.5;
_ELECTRONICS_CUTAWAY_TOP_CLEARANCE=5;
_ELECTRONICS_BAY_TOPWALL_THICKNESS=0.5;
_ELECTRONICS_CUTAWAY_WIDTH=60.0;
_ELECTRONICS_CUTAWAY_BACKWALL_EXTRAPERIMETERS=5;
_ELECTRONICS_CUTAWAY_THICKNESS=PHONE_THICKNESS+SCREEN_PROTECTOR_HEIGHT+KEYBOARD_PHONE_GAP_PERIMETERS*EXTRUSION_WIDTH-_ELECTRONICS_CUTAWAY_BACKWALL_EXTRAPERIMETERS*EXTRUSION_WIDTH;
_ELECTRONICS_CUTAWAY_LENGTH=25;
_KEYBOARD_THICKNESS=3.6;
_CASE_BOTTOM_LENGTH=38;
_CASE_KEYBOARD_Y_OVERLAP=EXTRUSION_WIDTH*2;

_USB_PORT_CUTAWAY_WIDTH=10;
_USB_PORT_CUTAWAY_HEIGHT=3.5;
_USB_PORT_CUTAWAY_LENGTH=20;
_USB_PORT_ROUNDING_RADIUS=2;

//Case cannot get smaller than 70mm, due to keyboard width
CASE_WIDTH=max(PHONE_WIDTH, 70.0) + EXTRUSION_WIDTH*PERIMETERS*2;
CASE_THICKNESS=PHONE_THICKNESS + EXTRUSION_WIDTH*CASE_BOTTOM_PERIMETERS + _KEYBOARD_THICKNESS + SCREEN_PROTECTOR_HEIGHT + KEYBOARD_PHONE_GAP_PERIMETERS*EXTRUSION_WIDTH;

assert(CASE_ROUNDING_RADIUS*2<PHONE_THICKNESS+2*CASE_BOTTOM_PERIMETERS*EXTRUSION_WIDTH, "Error: CASE_ROUNDING_RADIUS needs to be smaller than PHONE_THICKNESS + 2*CASE_BOTTOM_PERIMETERS*EXTRUSION_WIDTH");

include <include/roundedcube.scad>
include <include/roundedCorner.scad>

$fn=$preview ? 20 : 100;

module keyboardDummy() {
    color("darkgrey")
    translate([0,0.11,PHONE_THICKNESS/2 + SCREEN_PROTECTOR_HEIGHT + KEYBOARD_PHONE_GAP_PERIMETERS*EXTRUSION_WIDTH])
        import("import/BBQ10KBD.stl", convexity=5);
}

module usbPortCutaway() {
    translate([-_USB_PORT_CUTAWAY_WIDTH/2, -_USB_PORT_CUTAWAY_LENGTH/2, -_USB_PORT_CUTAWAY_HEIGHT/2-USB_PORT_Y_OFFSET])
    roundedcube(size=[_USB_PORT_CUTAWAY_WIDTH, _USB_PORT_CUTAWAY_LENGTH, _USB_PORT_CUTAWAY_HEIGHT], center=false, radius=_USB_PORT_ROUNDING_RADIUS, apply_to=["y"]);
}

module speakerHole() {
    translate([-PHONE_SPEAKER_CUTAWAY_WIDTH/2+PHONE_SPEAKER_CUTAWAY_DISTANCE_FROM_CENTER, -_USB_PORT_CUTAWAY_LENGTH/2, -_USB_PORT_CUTAWAY_HEIGHT/2])
    roundedcube(size=[PHONE_SPEAKER_CUTAWAY_WIDTH, _USB_PORT_CUTAWAY_LENGTH, _USB_PORT_CUTAWAY_HEIGHT], center=false, radius=_USB_PORT_ROUNDING_RADIUS, apply_to=["y"]);
}

module phoneDummy() {
    union() {
        if (PHONE_DUMMY_FILE==""){
            color("purple")
            translate([0,-PHONE_KEYBOARD_OVERLAP,0])
            union() {
                translate([-PHONE_WIDTH/2, 0, -PHONE_THICKNESS/2])
                roundedcube(size=[PHONE_WIDTH, 50+CASE_SLEEVE_LENGTH, PHONE_THICKNESS], center=false, radius=PHONE_ROUNDING_RADIUS, apply_to=["all"]);
                usbPortCutaway();
                speakerHole();
            }
        } else {
            color("purple")
            translate([0,-PHONE_KEYBOARD_OVERLAP,0])
            scale([1.015,1.00,1.11]) //TOLLERANCE
            import(PHONE_DUMMY_FILE, convexity=5);
            if (FORCE_HOLE_CUTAWAYS) {
                usbPortCutaway();
                speakerHole();
            }
        }
        if (SCREEN_PROTECTOR_HEIGHT>0) {
            color("blue")
            translate([-PHONE_WIDTH/2+PHONE_ROUNDING_RADIUS/2, -PHONE_KEYBOARD_OVERLAP+.15, PHONE_THICKNESS/2])
            cube(size=[PHONE_WIDTH-PHONE_ROUNDING_RADIUS, PHONE_KEYBOARD_OVERLAP, SCREEN_PROTECTOR_HEIGHT], center=false);
        }
    }
}

module phoneRoundingBox() {
    translate([-CASE_WIDTH/2,_CASE_KEYBOARD_Y_OVERLAP-_CASE_BOTTOM_LENGTH,-PHONE_THICKNESS/2 - EXTRUSION_WIDTH*CASE_BOTTOM_PERIMETERS])
    hull() {
        difference() {
            intersection() {
                roundedcube(size=[CASE_WIDTH,_CASE_BOTTOM_LENGTH+100, CASE_THICKNESS + CASE_TOP_ROUNDING_OFFSET], center=false, radius=CASE_ROUNDING_RADIUS, apply_to=["all"]);
                //translate([0,0,-50])
                
                roundedcube(size=[CASE_WIDTH,_CASE_BOTTOM_LENGTH+150, CASE_THICKNESS + CASE_TOP_ROUNDING_OFFSET + 50], center=false, radius=CASE_BOTTOM_ROUNDING_RADIUS, apply_to=["z"]);
                
                roundedcube(size=[CASE_WIDTH,_CASE_BOTTOM_LENGTH+150, CASE_THICKNESS + CASE_TOP_ROUNDING_OFFSET + 50], center=false, radius=CASE_BOTTOM_ROUNDING_RADIUS, apply_to=["x"]);
            }
            color("blue")
            roundedCornerNegative([CASE_BOTTOM_ROUNDING_RADIUS,CASE_BOTTOM_ROUNDING_RADIUS+5,CASE_BOTTOM_ROUNDING_RADIUS]);
            
            translate([CASE_WIDTH,0,0])
            mirror([1,0,0])
            color("blue")
            roundedCornerNegative([CASE_BOTTOM_ROUNDING_RADIUS,CASE_BOTTOM_ROUNDING_RADIUS+5,CASE_BOTTOM_ROUNDING_RADIUS]);
        }
    }
}

module case() {
    translate([-CASE_WIDTH/2, _CASE_KEYBOARD_Y_OVERLAP-_CASE_BOTTOM_LENGTH-1, -PHONE_THICKNESS/2 - EXTRUSION_WIDTH*CASE_BOTTOM_PERIMETERS])
    cube(size=[CASE_WIDTH, _CASE_BOTTOM_LENGTH, CASE_THICKNESS], center=false);
}

module caseSleeve() {
    difference() {
        translate([-CASE_WIDTH/2, -1, -PHONE_THICKNESS/2-0.05 - EXTRUSION_WIDTH*CASE_BOTTOM_PERIMETERS])
        roundedcube(size=[CASE_WIDTH, CASE_SLEEVE_LENGTH+1, PHONE_THICKNESS+EXTRUSION_WIDTH*(CASE_BOTTOM_PERIMETERS+CASE_SLEEVE_TOP_PERIMETERS)-0.05], center=false, radius=CASE_SLEEVE_ROUNDING_RADIUS, apply_to=["y"]);
        
        translate([-PHONE_WIDTH/2+PHONE_ROUNDING_RADIUS/2, -1.5, -0.05])
        cube(size=[PHONE_WIDTH-PHONE_ROUNDING_RADIUS, CASE_SLEEVE_LENGTH+3, PHONE_THICKNESS+EXTRUSION_WIDTH*CASE_BOTTOM_PERIMETERS*2-0.05], center=false);
    }
}

module caseSleeveLink() {
    scaleFactor = 2*CASE_THICKNESS/(PHONE_THICKNESS+EXTRUSION_WIDTH*CASE_BOTTOM_PERIMETERS*2-0.05);
    translate([0, CASE_SLEEVE_LINK_LENGTH-1.5, 0])
    intersection() {
        /*{
            translate([-CASE_WIDTH/2, -51, -PHONE_THICKNESS/2-0.05 - EXTRUSION_WIDTH*CASE_BOTTOM_PERIMETERS])
            roundedcube(size=[CASE_WIDTH, CASE_SLEEVE_LENGTH+100, 100+PHONE_THICKNESS+EXTRUSION_WIDTH*CASE_BOTTOM_PERIMETERS*2-0.05], center=false, radius=CASE_SLEEVE_ROUNDING_RADIUS, apply_to=["y"]);
        }*/
        {
            rotate([-90,0,180])
            linear_extrude(height=CASE_SLEEVE_LINK_LENGTH,scale=[1,scaleFactor],slices=100) {
                projection()
                rotate([90,0,0])
                caseSleeve();
            }
        }
        case();
    }
}

module electronicsCutaway() {
    
    union() {
        color("pink")
        translate([-_ELECTRONICS_CUTAWAY_WIDTH/2, -PHONE_KEYBOARD_OVERLAP-_ELECTRONICS_BAY_TOPWALL_THICKNESS-_ELECTRONICS_CUTAWAY_LENGTH, -PHONE_THICKNESS/2+0.01+_ELECTRONICS_CUTAWAY_BACKWALL_EXTRAPERIMETERS*EXTRUSION_WIDTH])
        cube(size=[_ELECTRONICS_CUTAWAY_WIDTH, _ELECTRONICS_CUTAWAY_LENGTH, _ELECTRONICS_CUTAWAY_THICKNESS], center=false);
        
        /*color("red")
        translate([-_ELECTRONICS_CUTAWAY_WIDTH/2+0.005,
            -_ELECTRONICS_CUTAWAY_TOP_CLEARANCE-_ELECTRONICS_CUTAWAY_LENGTH,
            PHONE_THICKNESS*.5+EXTRUSION_WIDTH])
        cube(size=[_ELECTRONICS_CUTAWAY_WIDTH-0.01, _ELECTRONICS_CUTAWAY_LENGTH, (KEYBOARD_PHONE_GAP_PERIMETERS*2-1) * EXTRUSION_WIDTH], center=false);*/
    }
}

module usbAnvilCutaway() {
    color("teal")
    translate([0,-PHONE_KEYBOARD_OVERLAP-7,-USB_PORT_Y_OFFSET-2])
    cube([35,13,2], center=true);
}

module usbAnvil() {
    translate([0,-PHONE_KEYBOARD_OVERLAP,-USB_PORT_Y_OFFSET])
    import("import/USB_Anvil.stl");
}

module screwM3Countersunk(length) {
    union() {
        cylinder(length,d1=3,d2=3,center=true);
        translate([0,0,-10])
        cylinder(2,d1=6.5,d2=3,center=true);
        translate([0,0,-16])
        cylinder(10,d1=6.5,d2=6.5,center=true);
    }
}

module screwM2(length) {
    union() {
        translate([0, 0, -length])
        cylinder(length,d1=2,d2=2,center=false);
        translate([0,0,-length-10])
        cylinder(10,d1=3.5,d2=3.5,center=false);
    }
}

module usbScrewHoles() {
    union() {
        translate([-10,-PHONE_KEYBOARD_OVERLAP-4.25,5])
        screwM3Countersunk(20);
        translate([10,-PHONE_KEYBOARD_OVERLAP-4.25,5])
        screwM3Countersunk(20);
    }
}

module keyboardScrewHoles() {
    thickness_correction_factor = -1.5-SCREEN_PROTECTOR_HEIGHT;
    screwLength = (CASE_THICKNESS+thickness_correction_factor>20 ? 20 :
                  CASE_THICKNESS+thickness_correction_factor>18 ? 18 :
                  CASE_THICKNESS+thickness_correction_factor>16 ? 16 :
                  //CASE_THICKNESS+thickness_correction_factor>14 ? 14 :
                  CASE_THICKNESS+thickness_correction_factor>12 ? 12 :
                  CASE_THICKNESS+thickness_correction_factor>10 ? 10 :
                  CASE_THICKNESS+thickness_correction_factor>8 ? 8 : 6);
    echo(str("\n\n\n->->-> Required keyboard M2 screw length: ", screwLength, "mm\n\n\n"));
    union() {
        translate([-27.5,-28.75,PHONE_THICKNESS/2 + KEYBOARD_PHONE_GAP_PERIMETERS*EXTRUSION_WIDTH + 3])
        screwM2(screwLength);
        translate([27.5,-28.75,PHONE_THICKNESS/2 + KEYBOARD_PHONE_GAP_PERIMETERS*EXTRUSION_WIDTH + 3])
        screwM2(screwLength);
    }
}

module keyboardCatches() {
    difference() {
        union() {
            scale([1,1.25,1])
            translate([-18.5, 0, _KEYBOARD_CATCH_DEPTH/2+PHONE_THICKNESS/2 + SCREEN_PROTECTOR_HEIGHT + KEYBOARD_PHONE_GAP_PERIMETERS*EXTRUSION_WIDTH - .01])
            cylinder(_KEYBOARD_CATCH_DEPTH,d1=3,d2=3,center=true);
            scale([1,1.25,1])
            translate([+18.5, 0, _KEYBOARD_CATCH_DEPTH/2+PHONE_THICKNESS/2 + SCREEN_PROTECTOR_HEIGHT + KEYBOARD_PHONE_GAP_PERIMETERS*EXTRUSION_WIDTH - .01])
            cylinder(_KEYBOARD_CATCH_DEPTH,d1=3,d2=3,center=true);
        }
        translate([-PHONE_WIDTH/2+PHONE_ROUNDING_RADIUS/2, 0.09, -0.05])
            cube(size=[PHONE_WIDTH-PHONE_ROUNDING_RADIUS, 5, PHONE_THICKNESS+EXTRUSION_WIDTH*CASE_BOTTOM_PERIMETERS*2-0.05], center=false);
    }
}

translate([0,0,_CASE_BOTTOM_LENGTH])
rotate([90,0,180])
intersection() {
    union() {
        difference() {
            union() {
                case();
                caseSleeveLink();
                caseSleeve();
            }
            keyboardDummy();
            phoneDummy();
            electronicsCutaway();
            usbScrewHoles();
            keyboardScrewHoles();
            usbAnvilCutaway();
        }
        usbAnvil();
        keyboardCatches();
    }
    phoneRoundingBox();
}

