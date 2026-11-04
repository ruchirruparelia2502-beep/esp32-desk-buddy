// ============================================================
//  IdleScreen.cpp — Black & White clock screen
// ============================================================

#include "IdleScreen.h"
#include "config.h"

// Pure B&W palette
static const uint32_t COL_BG       = 0x000000;  // pure black
static const uint32_t COL_TIME     = 0xFFFFFF;  // pure white
static const uint32_t COL_DATE     = 0x555555;  // mid-gray
static const uint32_t COL_DIVIDER  = 0x222222;
static const uint32_t COL_FOOTER   = 0x333333;

static uint16_t c(LGFX& d, uint32_t hex) {
  return d.color888((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
}

// ----------------------------------------------------------------
void IdleScreen::draw() {
  display.setRotation(1);
  display.fillScreen(c(display, COL_BG));

  String t, d;
  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    t = g_state.timeStr;
    d = g_state.dateStr;
    xSemaphoreGive(g_state.mutex);
  }

  drawTime(t);
  drawDate(d);
  drawDivider();
  drawFooter();
}

// ----------------------------------------------------------------
void IdleScreen::update(const String& prevTime, const String& prevDate) {
  String curTime, curDate;
  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    curTime = g_state.timeStr;
    curDate = g_state.dateStr;
    xSemaphoreGive(g_state.mutex);
  }

  if (curTime != prevTime) {
    display.fillRect(0, 20, 320, 95, c(display, COL_BG));
    drawTime(curTime);
  }
  if (curDate != prevDate) {
    display.fillRect(0, 115, 320, 35, c(display, COL_BG));
    drawDate(curDate);
  }
}

// ----------------------------------------------------------------
void IdleScreen::drawTime(const String& t) {
  String time = t.isEmpty() ? "--:--" : t;
  display.setTextDatum(lgfx::middle_center);
  display.setFont(&fonts::Font7);   // large 7-segment style
  display.setTextSize(1);
  display.setTextColor(c(display, COL_TIME));
  display.drawString(time.c_str(), 160, 72);
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
  display.drawFastHLine(140, 150, 40, c(display, COL_DIVIDER));
}

// ----------------------------------------------------------------
void IdleScreen::drawFooter() {
  display.setTextDatum(lgfx::middle_center);
  display.setFont(&fonts::Font2);
  display.setTextSize(1);
  display.setTextColor(c(display, COL_FOOTER));
  display.drawString("WAITING FOR MUSIC", 160, 210);
}
