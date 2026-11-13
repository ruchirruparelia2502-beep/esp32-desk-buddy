// ============================================================
//  TouchManager.cpp — CST820 capacitive touch wrapper (Direct I2C)
// ============================================================

#include "TouchManager.h"
#include <Wire.h>

#define CST820_I2C_ADDR 0x15

unsigned long TouchManager::_lastTapMs  = 0;
bool          TouchManager::_wasPressed = false;

// ----------------------------------------------------------------
bool TouchManager::begin() {
  if (PIN_TOUCH_RST >= 0) {
    pinMode(PIN_TOUCH_RST, OUTPUT);
    digitalWrite(PIN_TOUCH_RST, LOW);
    delay(20);
    digitalWrite(PIN_TOUCH_RST, HIGH);
    delay(50);
  }
  if (PIN_TOUCH_INT >= 0) {
    pinMode(PIN_TOUCH_INT, INPUT);
  }

  Wire.begin(PIN_TOUCH_SDA, PIN_TOUCH_SCL, 400000);
  Wire.setTimeOut(50);
  Serial.println("[Touch] CST820 I2C bus initialised");
  return true;
}

// ----------------------------------------------------------------
TouchPoint TouchManager::read() {
  TouchPoint tp;
  tp.pressed = false;

  // Request 6 bytes starting from register 0x01
  Wire.beginTransmission(CST820_I2C_ADDR);
  Wire.write(0x01);
  if (Wire.endTransmission(false) != 0) {
    return tp;
  }

  if (Wire.requestFrom((uint8_t)CST820_I2C_ADDR, (uint8_t)6) == 6) {
    uint8_t data[6];
    for (int i = 0; i < 6; i++) {
      data[i] = Wire.read();
    }

    uint8_t points = data[1] & 0x0F;
    if (points > 0) {
      uint16_t rx = ((data[2] & 0x0F) << 8) | data[3];
      uint16_t ry = ((data[4] & 0x0F) << 8) | data[5];

      tp.pressed = true;
      // CST820 raw is portrait 240x320; map to landscape 320x240 (rotation 1)
      tp.x = ry;
      tp.y = 239 - rx;

      tp.x = constrain(tp.x, 0, 319);
      tp.y = constrain(tp.y, 0, 239);
    }
  }

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
