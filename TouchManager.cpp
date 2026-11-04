// ============================================================
//  TouchManager.cpp — CST820 capacitive touch wrapper
// ============================================================

#include "TouchManager.h"
#include <Wire.h>
#include <TAMC_CST820.h>

static TAMC_CST820 _cst820(PIN_TOUCH_SDA, PIN_TOUCH_SCL,
                            PIN_TOUCH_INT, PIN_TOUCH_RST);

unsigned long TouchManager::_lastTapMs  = 0;
bool          TouchManager::_wasPressed = false;

// ----------------------------------------------------------------
bool TouchManager::begin() {
  // TAMC_CST820 initialises Wire internally
  _cst820.begin();
  _cst820.setRotation(ROTATION_INVERTED);  // matches landscape rotation=1
  Serial.println("[Touch] CST820 initialised");
  return true;
}

// ----------------------------------------------------------------
TouchPoint TouchManager::read() {
  TouchPoint tp;
  _cst820.read();

  if (_cst820.isFingerUp()) {
    tp.pressed = false;
    return tp;
  }

  tp.pressed = true;

  // CST820 raw is portrait 240x320; map to landscape 320x240
  // Rotation 1 (landscape): swapped + possible mirror
  // Raw: (rx, ry) in portrait 0..239 x 0..319
  // Landscape: lx = ry, ly = 239 - rx
  int rx = _cst820.x;
  int ry = _cst820.y;

  tp.x = ry;
  tp.y = 239 - rx;

  // Clamp
  tp.x = constrain(tp.x, 0, 319);
  tp.y = constrain(tp.y, 0, 239);

  return tp;
}

// ----------------------------------------------------------------
bool TouchManager::isTapped(TouchPoint& out) {
  out = read();

  unsigned long now = millis();

  if (out.pressed && !_wasPressed) {
    // Fresh press — check debounce
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
