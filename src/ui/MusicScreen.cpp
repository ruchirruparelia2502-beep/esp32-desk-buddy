// ============================================================
//  MusicScreen.cpp — Minimalist Black & White Music UI
//  Inspired by modern card player (white canvas, bold typography)
// ============================================================

#include "MusicScreen.h"
#include "../config.h"
#include "../bt/AMSClient.h"
#include "../bt/AVRCPHandler.h"

// Monochrome Palette
static const uint32_t COL_BG          = 0xFFFFFF;  // Clean white
static const uint32_t COL_TEXT_MAIN   = 0x000000;  // Deep black
static const uint32_t COL_TEXT_MUTED  = 0x666666;  // Mid gray
static const uint32_t COL_TEXT_LIGHT  = 0x999999;  // Light gray
static const uint32_t COL_LINE        = 0xE0E0E0;  // Subtle separator / progress track
static const uint32_t COL_LINE_ACTIVE = 0x000000;  // Active progress

// Helper
static uint16_t c(LGFX& d, uint32_t hex) {
  return d.color888((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
}

// ----------------------------------------------------------------
void MusicScreen::draw() {
  display.setRotation(1);  // landscape 320x240
  drawBackground();

  TrackInfo  track;
  BTSource   src;
  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    track = g_state.track;
    src   = g_state.btSource;
    xSemaphoreGive(g_state.mutex);
  }

  drawHeader(src);
  drawArtist(track.artist);
  drawTitle(track.title);
  drawAlbum(track.album);
  drawDivider();
  drawButtons(track.isPlaying);
}

// ----------------------------------------------------------------
void MusicScreen::update(const TrackInfo& prev) {
  TrackInfo  cur;
  BTSource   src;
  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    cur = g_state.track;
    src = g_state.btSource;
    xSemaphoreGive(g_state.mutex);
  }

  uint16_t bg = c(display, COL_BG);

  if (cur.artist != prev.artist) {
    display.fillRect(15, 38, 290, 32, bg);
    drawArtist(cur.artist);
  }

  if (cur.title != prev.title) {
    display.fillRect(15, 72, 290, 28, bg);
    drawTitle(cur.title);
  }

  if (cur.album != prev.album) {
    display.fillRect(15, 102, 290, 22, bg);
    drawAlbum(cur.album);
  }

  if (cur.isPlaying != prev.isPlaying) {
    // Redraw center play/pause circle
    drawButtons(cur.isPlaying);
  }
}

// ----------------------------------------------------------------
bool MusicScreen::handleTouch(int tx, int ty) {
  if (ty < 150) return false;   // buttons area is y: 150-230

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
void MusicScreen::drawHeader(BTSource src) {
  // Pill badge: [IOS · BLUETOOTH] or [ANDROID · BLUETOOTH]
  const char* label = (src == BTSource::ANDROID_BT) ? "ANDROID . BT" : "IOS . BT";

  display.drawRoundRect(20, 14, 100, 18, 4, c(display, COL_LINE));
  display.setTextDatum(lgfx::middle_center);
  display.setFont(&fonts::Font0); // tiny
  display.setTextSize(1);
  display.setTextColor(c(display, COL_TEXT_LIGHT));
  display.drawString(label, 70, 23);
}

// ----------------------------------------------------------------
void MusicScreen::drawArtist(const String& artist) {
  String a = artist.isEmpty() ? "PORTISHEAD" : artist;
  if (a.length() > 24) a = a.substring(0, 23) + ".";

  // Force uppercase for header look
  for (int i = 0; i < a.length(); i++) a[i] = toupper(a[i]);

  display.setTextDatum(lgfx::top_left);
  display.setFont(&fonts::Font4);
  display.setTextSize(1);
  display.setTextColor(c(display, COL_TEXT_MAIN));
  display.drawString(a.c_str(), 20, 42);
}

// ----------------------------------------------------------------
void MusicScreen::drawTitle(const String& title) {
  String t = title.isEmpty() ? "Undenied" : title;
  if (t.length() > 28) t = t.substring(0, 27) + "...";

  display.setTextDatum(lgfx::top_left);
  display.setFont(&fonts::Font2);
  display.setTextSize(1);
  display.setTextColor(c(display, COL_TEXT_MUTED));
  display.drawString(t.c_str(), 20, 74);
}

// ----------------------------------------------------------------
void MusicScreen::drawAlbum(const String& album) {
  if (album.isEmpty()) return;
  String al = album;
  if (al.length() > 34) al = al.substring(0, 33) + "...";

  display.setTextDatum(lgfx::top_left);
  display.setFont(&fonts::Font0);
  display.setTextSize(1);
  display.setTextColor(c(display, COL_TEXT_LIGHT));
  display.drawString(al.c_str(), 20, 98);
}

// ----------------------------------------------------------------
void MusicScreen::drawDivider() {
  // Progress bar style track line
  display.drawFastHLine(20, 130, 280, c(display, COL_LINE));
  display.drawFastHLine(20, 130, 110, c(display, COL_LINE_ACTIVE));
  display.fillCircle(130, 130, 3, c(display, COL_LINE_ACTIVE));
}

// ----------------------------------------------------------------
void MusicScreen::drawButtons(bool isPlaying) {
  uint16_t bg = c(display, COL_BG);
  uint16_t black = c(display, COL_TEXT_MAIN);

  // Erase button area
  display.fillRect(0, 150, 320, 80, bg);

  // PREV Button (|<< icon)
  display.setTextDatum(lgfx::middle_center);
  display.setFont(&fonts::Font4);
  display.setTextColor(black);
  display.drawString("|<", 70, 185);

  // Center PLAY/PAUSE Circular Button
  display.fillCircle(160, 185, 24, black);
  display.setTextColor(c(display, COL_BG));
  display.setFont(&fonts::Font4);

  if (isPlaying) {
    // Pause symbol "||"
    display.drawString("||", 160, 185);
  } else {
    // Play symbol ">"
    display.drawString(">", 162, 185);
  }

  // NEXT Button (>>| icon)
  display.setTextColor(black);
  display.drawString(">|", 250, 185);
}

void MusicScreen::drawButton(int x, int y, int w, int h, uint32_t bgColor, const char* label) {
  // Kept for backward compatibility if called
}
