// ============================================================
//  TouchManager.cpp — LovyanGFX integrated CST820 touch handler
// ============================================================

#include "TouchManager.h"
#include "DisplaySetup.h"

unsigned long TouchManager::_lastTapMs  = 0;
bool          TouchManager::_wasPressed = false;

// ----------------------------------------------------------------
bool TouchManager::begin() {
  Serial.println("[Touch] CST820 initialized via LovyanGFX");
  return true;
}

// ----------------------------------------------------------------
TouchPoint TouchManager::read() {
  TouchPoint tp;
  int32_t x = 0, y = 0;
  if (display.getTouch(&x, &y)) {
    tp.pressed = true;
    tp.x = (int)x;
    tp.y = (int)y;
  } else {
    tp.pressed = false;
  }
  return tp;
}

// ----------------------------------------------------------------
bool TouchManager::isTapped(TouchPoint& out) {
  out = read();

  unsigned long now = millis();

  if (out.pressed && !_wasPressed) {
    if ((now - _lastTapMs) > TOUCH_DEBOUNCE_MS) {
      _lastTapMs  = now;
      _wasPressed = true;
      return true;
    }
  }

  if (!out.pressed) {
    _wasPressed = false;
  }

  return false;
}
