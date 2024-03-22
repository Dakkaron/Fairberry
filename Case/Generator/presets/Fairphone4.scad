//Phone dimensions in mm
PHONE_WIDTH=75.5;
PHONE_THICKNESS=11.5;
PHONE_KEYBOARD_OVERLAP=11.5; // Determines how much the keyboard will overlap the bottom of the phone
USB_PORT_Y_OFFSET=0.4;

GENERATE_TOP_CASE=false; // If set to false, will generate the keyboard case. If set to true, will instead create the top case

// Leave empty ("") to generate the phone shape using this script
PHONE_DUMMY_FILE="import/FP4_Dummy_Screenprotector.stl";

// Parameters for a speaker hole. Only relevant if PHONE_DUMMY_FILE=="" or FORCE_HOLE_CUTAWAYS=true
PHONE_SPEAKER_CUTAWAY_WIDTH=22;
PHONE_SPEAKER_CUTAWAY_DISTANCE_FROM_CENTER=18;

// Corner radius of the phone. Only relevant if PHONE_DUMMY_FILE==""
PHONE_ROUNDING_RADIUS=5;

PERIMETERS=4; // How many perimeters should the top part of the case be thick?

// How many perimeters should the case be thick below the phone?
CASE_BOTTOM_PERIMETERS=3;

CASE_SLEEVE_TOP_PERIMETERS=2;//CASE_BOTTOM_PERIMETERS;
CASE_SLEEVE_LENGTH=28.5; // Length of the case extension towards the top of the phone
CASE_SLEEVE_LINK_LENGTH=5;
CASE_SLEEVE_ROUNDING_RADIUS=4; // Corner radius of the sleve part of the case.
CASE_ROUNDING_RADIUS=4; // Corner radius of the case.
CASE_BOTTOM_ROUNDING_RADIUS=10;
CASE_TOP_ROUNDING_OFFSET=1; // Increasing this number makes the top less rounded

KEYBOARD_PHONE_GAP_PERIMETERS=2; // Thickness of the separator between keyboard and phone screen

KEYBOARD_CATCH_DEPTH=1.5;

FORCE_HOLE_CUTAWAYS=true;