#pragma once

// ============================================================
//  TouchManager.h — CST820 capacitive touch wrapper
// ============================================================

#include <Arduino.h>
#include "config.h"

struct TouchPoint {
  int x  = 0;
  int y  = 0;
  bool pressed = false;
};

class TouchManager {
public:
  /**
   * Initialise I2C and CST820 driver. Call once from setup().
   */
  static bool begin();

  /**
   * Poll the touch controller. Returns the current touch state.
   * Maps raw coordinates to landscape (320x240) display coordinates.
   */
  static TouchPoint read();

  /**
   * Returns true if a NEW press was detected since last call
   * (debounced by TOUCH_DEBOUNCE_MS).
   */
  static bool isTapped(TouchPoint& out);

private:
  static unsigned long _lastTapMs;
  static bool          _wasPressed;
};
