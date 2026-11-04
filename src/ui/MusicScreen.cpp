// ============================================================
//  MusicScreen.cpp — Minimalist Funky OLED Black Music UI
//  No badges, pure typography, high-contrast monochrome design
// ============================================================

#include "MusicScreen.h"
#include "../config.h"
#include "../bt/AMSClient.h"
#include "../bt/AVRCPHandler.h"

// OLED Minimal Palette
static const uint32_t COL_BG          = 0x000000;  // Pure OLED black
static const uint32_t COL_TEXT_MAIN   = 0xFFFFFF;  // Pure crisp white
static const uint32_t COL_TEXT_SUB    = 0xA1A1AA;  // Muted light zinc
static const uint32_t COL_LINE_BG     = 0x27272A;  // Dark scrubber track
static const uint32_t COL_LINE_ACTIVE = 0xFFFFFF;  // Bright scrubber fill

static uint16_t c(LGFX& d, uint32_t hex) {
  return d.color888((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
}

// ----------------------------------------------------------------
void MusicScreen::draw() {
  display.setRotation(1);  // landscape 320x240
  drawBackground();

  TrackInfo track;
  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    track = g_state.track;
    xSemaphoreGive(g_state.mutex);
  }

  drawArtist(track.artist);
  drawTitle(track.title);
  drawScrubber();
  drawButtons(track.isPlaying);
}

// ----------------------------------------------------------------
void MusicScreen::update(const TrackInfo& prev) {
  TrackInfo cur;
  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    cur = g_state.track;
    xSemaphoreGive(g_state.mutex);
  }

  uint16_t bg = c(display, COL_BG);

  if (cur.artist != prev.artist) {
    display.fillRect(20, 26, 280, 42, bg);
    drawArtist(cur.artist);
  }

  if (cur.title != prev.title) {
    display.fillRect(20, 72, 280, 32, bg);
    drawTitle(cur.title);
  }

  if (cur.isPlaying != prev.isPlaying) {
    drawButtons(cur.isPlaying);
  }
}

// ----------------------------------------------------------------
bool MusicScreen::handleTouch(int tx, int ty) {
  if (ty < 150) return false;   // buttons area: y 150..235

  BTSource src;
  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(20)) == pdTRUE) {
    src = g_state.btSource;
    xSemaphoreGive(g_state.mutex);
  }

  if (tx < 110) {
    // PREV
    if (src == BTSource::IOS_BLE) {
      AMSClient::sendCommand(AMS_CMD_PREV);
    } else if (src == BTSource::ANDROID_BT) {
      AVRCPHandler::prev();
    }
    return true;
  } else if (tx > 210) {
    // NEXT
    if (src == BTSource::IOS_BLE) {
      AMSClient::sendCommand(AMS_CMD_NEXT);
    } else if (src == BTSource::ANDROID_BT) {
      AVRCPHandler::next();
    }
    return true;
  } else {
    // PLAY / PAUSE
    if (src == BTSource::IOS_BLE) {
      AMSClient::sendCommand(AMS_CMD_TOGGLE_PLAY);
    } else if (src == BTSource::ANDROID_BT) {
      AVRCPHandler::togglePlayPause();
    }
    return true;
  }
}

// ----------------------------------------------------------------
void MusicScreen::drawBackground() {
  display.fillScreen(c(display, COL_BG));
}

// ----------------------------------------------------------------
void MusicScreen::drawArtist(const String& artist) {
  String a = artist.isEmpty() ? "PORTISHEAD" : artist;
  if (a.length() > 20) a = a.substring(0, 19) + ".";

  // Bold uppercase stylized headline
  for (int i = 0; i < a.length(); i++) a[i] = toupper(a[i]);

  display.setTextDatum(lgfx::top_left);
  display.setFont(&fonts::Font4);
  display.setTextSize(1);
  display.setTextColor(c(display, COL_TEXT_MAIN));
  display.drawString(a.c_str(), 24, 32);
}

// ----------------------------------------------------------------
void MusicScreen::drawTitle(const String& title) {
  String t = title.isEmpty() ? "Undenied" : title;
  if (t.length() > 28) t = t.substring(0, 27) + "...";

  display.setTextDatum(lgfx::top_left);
  display.setFont(&fonts::Font2);
  display.setTextSize(1);
  display.setTextColor(c(display, COL_TEXT_SUB));
  display.drawString(t.c_str(), 24, 76);
}

// ----------------------------------------------------------------
void MusicScreen::drawScrubber() {
  // Ultra-minimal hairline progress track
  display.drawFastHLine(24, 128, 272, c(display, COL_LINE_BG));
  display.drawFastHLine(24, 128, 105, c(display, COL_LINE_ACTIVE));
  display.fillCircle(129, 128, 3, c(display, COL_LINE_ACTIVE));
}

// ----------------------------------------------------------------
void MusicScreen::drawButtons(bool isPlaying) {
  uint16_t bg = c(display, COL_BG);
  uint16_t white = c(display, COL_TEXT_MAIN);

  // Clear button zone
  display.fillRect(0, 150, 320, 85, bg);

  // PREV Icon
  display.setTextDatum(lgfx::middle_center);
  display.setFont(&fonts::Font4);
  display.setTextColor(white);
  display.drawString("|<", 68, 190);

  // Center Tactile Circle Button
  display.fillCircle(160, 190, 24, white);
  display.setTextColor(c(display, COL_BG));
  display.setFont(&fonts::Font4);

  if (isPlaying) {
    display.drawString("||", 160, 190);
  } else {
    display.drawString(">", 162, 190);
  }

  // NEXT Icon
  display.setTextColor(white);
  display.drawString(">|", 252, 190);
}
