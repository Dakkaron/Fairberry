/*
 * LED state display. Each half of the keyboard LEDs is driven by one rule taken from the
 * active keyboard state, applied as an envelope on top of the brightness the user has set, so
 * a pattern never overwrites the values the Android app edits. Only one state can be active
 * at a time, so a state owns both halves.
 * Pin 13 is the left half, pin 11 the right half.
 */

#ifndef FAIRBERRY_STATUSLEDS_H
#define FAIRBERRY_STATUSLEDS_H

#include "keymaps.h"

#ifdef STATUS_LED_PATTERNS

#define LED_HALF_LEFT 0
#define LED_HALF_RIGHT 1

#define LED_HALF_OFF 0
#define LED_HALF_SOLID 1
#define LED_HALF_PULSE 2

#define LED_STATE_PLAIN 0
#define LED_STATE_CURSOR 1
#define LED_STATE_SYM_LOCK 2
#define LED_STATE_CAPS_LOCK 3
#define LED_STATE_CTRL_LOCK 4
#define LED_STATE_ALT_LOCK 5

struct LedHalfRule {
  uint8_t kind;
  unsigned long periodMs;
  bool halfPhase;
};

// Entering a state or a lock clears the previous sticky states, so the order below is
// only a stable tie break, not a priority the keyboard can actually reach.
static inline int activeLedState(bool cursorMode, bool symLock, bool capsLock, bool ctrlLock, bool altLock) {
  if (cursorMode) return LED_STATE_CURSOR;
  if (symLock) return LED_STATE_SYM_LOCK;
  if (capsLock) return LED_STATE_CAPS_LOCK;
  if (ctrlLock) return LED_STATE_CTRL_LOCK;
  if (altLock) return LED_STATE_ALT_LOCK;
  return LED_STATE_PLAIN;
}

static inline LedHalfRule ledRuleFor(int state, int half) {
  switch (state) {
    case LED_STATE_CURSOR:
      return { LED_HALF_PULSE, LED_PULSE_PERIOD_MS, false };
    case LED_STATE_SYM_LOCK:
      return { LED_HALF_PULSE, LED_PULSE_PERIOD_MS, half == LED_HALF_RIGHT };
    case LED_STATE_CAPS_LOCK:
      return half == LED_HALF_LEFT
        ? LedHalfRule{ LED_HALF_PULSE, LED_PULSE_PERIOD_MS, false }
        : LedHalfRule{ LED_HALF_SOLID, 0, false };
    case LED_STATE_CTRL_LOCK:
      return half == LED_HALF_RIGHT
        ? LedHalfRule{ LED_HALF_PULSE, LED_PULSE_PERIOD_MS, false }
        : LedHalfRule{ LED_HALF_OFF, 0, false };
    case LED_STATE_ALT_LOCK:
      return { LED_HALF_PULSE, LED_FAST_PULSE_PERIOD_MS, half == LED_HALF_RIGHT };
    default:
      return { LED_HALF_SOLID, 0, false };
  }
}

/*
 * Triangle ramp over the period instead of a hard on/off, so a pulse reads as
 * breathing and the eye can tell two different states apart while both halves are
 * lit at the same moment.
 */
static inline uint8_t ledHalfEnvelope(const LedHalfRule& rule, unsigned long phaseMs) {
  if (rule.kind == LED_HALF_OFF) return 0;
  if (rule.kind == LED_HALF_SOLID) return 255;

  unsigned long t = (phaseMs + (rule.halfPhase ? rule.periodMs / 2 : 0)) % rule.periodMs;
  unsigned long value = t * 510UL / rule.periodMs;
  if (t >= rule.periodMs / 2) value = 510 - value;
  return (uint8_t)value;
}

#endif

#endif
