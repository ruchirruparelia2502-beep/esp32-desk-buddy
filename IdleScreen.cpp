// ============================================================
//  IdleScreen.cpp — Minimalist Funky Clock Screen
//  Supports 24-hour and 12-hour AM/PM formats (touch to toggle)
// ============================================================

#include "IdleScreen.h"
#include "config.h"
#include "TimeSync.h"

// Pure B&W OLED palette
static const uint32_t COL_BG       = 0x000000;  // pure black
static const uint32_t COL_TIME     = 0xFFFFFF;  // pure white
static const uint32_t COL_DATE     = 0x71717A;  // zinc gray
static const uint32_t COL_AMPM     = 0xA1A1AA;  // muted white
static const uint32_t COL_DIVIDER  = 0x27272A;
static const uint32_t COL_FOOTER   = 0x52525B;

static uint16_t c(LGFX& d, uint32_t hex) {
  return d.color888((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
}

// ----------------------------------------------------------------
void IdleScreen::draw() {
  display.setRotation(1);
  display.fillScreen(c(display, COL_BG));

  String t, d, ampm;
  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    t = g_state.timeStr;
    d = g_state.dateStr;
    ampm = g_state.ampmStr;
    xSemaphoreGive(g_state.mutex);
  }

  drawTime(t, ampm);
  drawDate(d);
  drawDivider();
  drawFooter();
}

// ----------------------------------------------------------------
void IdleScreen::update(const String& prevTime, const String& prevDate) {
  String curTime, curDate, curAmpm;
  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    curTime = g_state.timeStr;
    curDate = g_state.dateStr;
    curAmpm = g_state.ampmStr;
    xSemaphoreGive(g_state.mutex);
  }

  if (curTime != prevTime) {
    display.fillRect(0, 20, 320, 95, c(display, COL_BG));
    drawTime(curTime, curAmpm);
  }
  if (curDate != prevDate) {
    display.fillRect(0, 115, 320, 35, c(display, COL_BG));
    drawDate(curDate);
  }
}

// ----------------------------------------------------------------
bool IdleScreen::handleTouch(int tx, int ty) {
  // Toggle between 12H and 24H on any tap on the idle screen!
  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    g_state.is24Hour = !g_state.is24Hour;
    xSemaphoreGive(g_state.mutex);
  }

  // Force update time strings immediately and redraw
  TimeSync::forceRefresh();
  draw();
  Serial.printf("[Clock] Toggled time format: %s\n", g_state.is24Hour ? "24-Hour" : "12-Hour");
  return true;
}

// ----------------------------------------------------------------
void IdleScreen::drawTime(const String& t, const String& ampm) {
  String time = t.isEmpty() ? "--:--" : t;

  display.setTextDatum(lgfx::middle_center);
  display.setFont(&fonts::Font7);   // large 7-segment / digital style
  display.setTextSize(1);
  display.setTextColor(c(display, COL_TIME));

  if (ampm.isEmpty()) {
    // 24-hour centered
    display.drawString(time.c_str(), 160, 72);
  } else {
    // 12-hour: slightly shifted left with elegant AM/PM badge
    display.drawString(time.c_str(), 144, 72);

    display.setTextDatum(lgfx::middle_left);
    display.setFont(&fonts::Font2);
    display.setTextColor(c(display, COL_AMPM));
    display.drawString(ampm.c_str(), 232, 74);
  }
}

// ----------------------------------------------------------------
void IdleScreen::drawDate(const String& d) {
  String date = d.isEmpty() ? "" : d;
  display.setTextDatum(lgfx::middle_center);
  display.setFont(&fonts::Font2);
  display.setTextSize(1);
  display.setTextColor(c(display, COL_DATE));

  // Uppercase via character iteration
  String upper = date;
  for (int i = 0; i < upper.length(); i++) upper[i] = toupper(upper[i]);
  display.drawString(upper.c_str(), 160, 128);
}

// ----------------------------------------------------------------
void IdleScreen::drawDivider() {
  // Short centered divider line
  display.drawFastHLine(140, 155, 40, c(display, COL_DIVIDER));
}

// ----------------------------------------------------------------
void IdleScreen::drawFooter() {
  display.setTextDatum(lgfx::middle_center);
  display.setFont(&fonts::Font0); // tiny
  display.setTextSize(1);
  display.setTextColor(c(display, COL_FOOTER));
  display.drawString("WAITING FOR AUDIO", 160, 210);
}
