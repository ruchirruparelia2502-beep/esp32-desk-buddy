// ============================================================
//  TouchManager.cpp — Universal CST820 / CST816 capacitive touch
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

  Wire.begin(PIN_TOUCH_SDA, PIN_TOUCH_SCL);
  Wire.setTimeOut(30);
  Serial.printf("[Touch] I2C Wire started on SDA=%d, SCL=%d, RST=%d\n",
                PIN_TOUCH_SDA, PIN_TOUCH_SCL, PIN_TOUCH_RST);
  return true;
}

// ----------------------------------------------------------------
TouchPoint TouchManager::read() {
  TouchPoint tp;
  tp.pressed = false;

  Wire.beginTransmission(CST820_I2C_ADDR);
  Wire.write(0x00);
  if (Wire.endTransmission() != 0) {
    return tp;
  }

  if (Wire.requestFrom((uint8_t)CST820_I2C_ADDR, (uint8_t)7) == 7) {
    uint8_t buf[7];
    for (int i = 0; i < 7; i++) {
      buf[i] = Wire.read();
    }

    uint8_t points = 0;
    uint16_t rx = 0, ry = 0;

    // Check Reg 0x02 (points in CST816S/CST820 standard)
    if ((buf[2] & 0x0F) > 0) {
      points = buf[2] & 0x0F;
      rx = ((buf[3] & 0x0F) << 8) | buf[4];
      ry = ((buf[5] & 0x0F) << 8) | buf[6];
    }
    // Check Reg 0x01 (alternative register layout)
    else if ((buf[1] & 0x0F) > 0) {
      points = buf[1] & 0x0F;
      rx = ((buf[2] & 0x0F) << 8) | buf[3];
      ry = ((buf[4] & 0x0F) << 8) | buf[5];
    }

    if (points > 0 && rx < 400 && ry < 400) {
      tp.pressed = true;
      // CST820 raw portrait 240x320 -> Landscape 320x240 (rotation 1)
      tp.x = constrain((int)ry, 0, 319);
      tp.y = constrain(239 - (int)rx, 0, 239);
    }
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
