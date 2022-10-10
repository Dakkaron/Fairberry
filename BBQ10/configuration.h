/*
 * ## Mainboard type
 * Can be either of the following:
 * FAIRBERRY_V0_2_0 : This is the custom mainboard, version 0.2.0
 * FAIRBERRY_V0_1_1 : This is the custom mainboard, version 0.1.1
 * ARDUINO : The original option, using an Arduino Pro Micro
 * BEETLE : A slightly different version of the original option, using a smaller Beetle board based on the ATMega32u4. Not recommeded, because the beetle has one to few pins, so you have to solder directly to the ATMega32u4.
 * ESP32: Using an ESP32-based board to couple the board using Bluetooth. Not recommended, because the power options don't really work right now, and it consumes a lot of power.
 */
#define BOARD_TYPE FAIRBERRY_V0_2_0

/*
 * ## Power Saving
 * Without power saving options the whole device consumes around 60mA.
 * While it's active, that's fine, but it consumes the same amount of
 * energy while it's idle. Since the device is idle for most of the time,
 * reducing the idle power is important.
 * 
 * For this there are multiple options:
 * 
 * ### IDLE_TIMEOUT
 * This is the basic setting that determines after how many ms
 * powersaving features are triggered. If the device was idle for more
 * than the given time, the keyboard backlight is disabled and the given
 * power save mode is applied (if any).
 * 
 * ### POWERSAVE_ARDUINO_IDLE
 * This is the recommended powersaving mode for BOARD_TYPE ARDUINO. It's a light sleep that keeps
 * the USB connection active. It saves around 40mA. It introduces a delay
 * of up to 120ms when the device is woken up.
 * 
 * ### POWERSAVE_ARDUINO_POWERDOWN
 * This is the more aggressive powersaving method for BOARD_TYPE ARDUINO. It completely turns off
 * the microcontroller, saving around 55mA. This takes the power consumption
 * down to almost nothing, but USB needs to reconnect after the keyboard
 * is woken up. This takes about 1-2 seconds.
 * 
 * ### POWERSAVE_ESP32_LOGHT_SLEEP
 * This is the recommended powersaving mode for BOARD_TYPE ARDUINO. It's a light sleep that keeps
 * the Bluetooth connection active. It reduces the power consumption from around 160mA to about 20mA when not active.
 * It introduces a delay of up to 50ms when the device is woken up.
 * 
 * For Arduino boards you can use either POWERSAVE_ARDUINO_IDLE or POWERSAVE_ARDUINO_POWERDOWN or none, but not both.
 * 
 * For ESP32 boards you can either use POWERSAVE_ESP32_LIGHT_SLEEP or none.
 */
#define IDLE_TIMEOUT 60L * 1000L
#define POWERSAVE_ARDUINO_IDLE
//#define POWERSAVE_ARDUINO_CLOCKDOWN
//#define POWERSAVE_ARDUINO_POWERDOWN
//#define POWERSAVE_ESP32_LIGHT_SLEEP
//#define POWERSAVE_ESP32_DEEP_SLEEP

//#define ARDUINO_CLOCKDOWN_DIVISION 16


/*
 * To visually show that the device is in cursor mode, the keyboard backlight
 * will blink rapidly if BLINK_IN_CURSOR_MODE is set. Change BLINK_TIMEOUT
 * to change the blinking frequency.
 * 
 * If you are using a fairberry mainboard, you can use LED0_IN_CURSOR_MDOE
 * instead of BLINK_IN_CURSOR_MODE, to have the LED0 glow when in cursor mode (doesn't blink).
 */
//#define BLINK_IN_CURSOR_MODE
#define BLINK_TIMEOUT 20L
#define LED0_IN_CURSOR_MODE

/*
 * 
 */
#define STICKY_SYM
#define STICKY_LSH
#define STICKY_RSH
#define STICKY_CTRL
#define STICKY_ALT
#define DOUBLETAP_LOCK_SYM
#define DOUBLETAP_LOCK_LSH
#define DOUBLETAP_LOCK_RSH
//#define DOUBLETAP_LOCK_CTRL
//#define DOUBLETAP_LOCK_ALT

#define DEBOUNCE_LOOPS 20

/*
 * If you define DEBUG_SERIAL_INSTEAD_OF_USB, the device will not function as a keyboard, but instead
 * output key presses to serial. Used for debugging.
 */
//#define DEBUG_SERIAL_INSTEAD_OF_USB
