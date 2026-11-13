// ============================================================
//  TouchManager.cpp — CST820 driver powered by bb_captouch
// ============================================================

#include "TouchManager.h"
#include "bb_captouch.h"

static BBCapTouch _bbTouch;
unsigned long TouchManager::_lastTapMs  = 0;
bool          TouchManager::_wasPressed = false;

// ----------------------------------------------------------------
bool TouchManager::begin() {
  if (PIN_TOUCH_RST >= 0) {
    pinMode(PIN_TOUCH_RST, OUTPUT);
    digitalWrite(PIN_TOUCH_RST, LOW);
    delay(50);
    digitalWrite(PIN_TOUCH_RST, HIGH);
    delay(100);
  }

  _bbTouch.init(TOUCH_CYD_24C);
  _bbTouch.setOrientation(90, 320, 240);
  Serial.printf("[Touch] bb_captouch initialized (sensor type %d)\n",
                _bbTouch.sensorType());
  return true;
}

// ----------------------------------------------------------------
TouchPoint TouchManager::read() {
  TouchPoint tp;
  TOUCHINFO ti;

  if (_bbTouch.getSamples(&ti) && ti.count > 0) {
    tp.pressed = true;
    tp.x = constrain((int)ti.x[0], 0, 319);
    tp.y = constrain((int)ti.y[0], 0, 239);
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
