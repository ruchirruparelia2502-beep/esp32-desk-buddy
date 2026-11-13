// ============================================================
//  TouchManager.cpp — LovyanGFX integrated XPT2046 touch driver
// ============================================================

#include "TouchManager.h"
#include "DisplaySetup.h"

unsigned long TouchManager::_lastTapMs  = 0;
bool          TouchManager::_wasPressed = false;

// ----------------------------------------------------------------
bool TouchManager::begin() {
  Serial.println("[Touch] XPT2046 touch driver initialized via LovyanGFX");
  return true;
}

// ----------------------------------------------------------------
TouchPoint TouchManager::read() {
  TouchPoint tp;
  int32_t x = 0, y = 0;
  if (display.getTouch(&x, &y)) {
    tp.pressed = true;
    tp.x = constrain((int)x, 0, 319);
    tp.y = constrain((int)y, 0, 239);
  } else {
    tp.pressed = false;
  }
  return tp;
}

// ----------------------------------------------------------------
bool TouchManager::isTapped(TouchPoint& out) {
  out = read();
  unsigned long now = millis();

  if (out.pressed) {
    if (!_wasPressed && ((now - _lastTapMs) > TOUCH_DEBOUNCE_MS)) {
      _lastTapMs  = now;
      _wasPressed = true;
      Serial.printf("[Touch] Tap detected at (%d, %d)\n", out.x, out.y);
      return true;
    }
  } else {
    _wasPressed = false;
  }

  return false;
}
