// ============================================================
//  MusicScreen.cpp — Minimalist Funky OLED Black Music UI
//  Auto-fitting typography: BOTH Artist & Title ALWAYS fit!
//  Live dynamic progress bar + timestamps
// ============================================================

#include "MusicScreen.h"
#include "config.h"
#include "AMSClient.h"
#include "AVRCPHandler.h"

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

  uint32_t elapsed = track.positionMs;
  if (track.isPlaying && track.lastPosUpdateMs > 0) {
    elapsed += (millis() - track.lastPosUpdateMs);
  }
  if (track.durationMs > 0 && elapsed > track.durationMs) {
    elapsed = track.durationMs;
  }

  drawArtist(track.artist);
  drawTitle(track.title);
  drawScrubber(elapsed, track.durationMs);
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

  // If artist or title changes, clear the combined meta area
  if (cur.artist != prev.artist || cur.title != prev.title) {
    display.fillRect(20, 24, 280, 88, bg);
    drawArtist(cur.artist);
    drawTitle(cur.title);
  }

  if (cur.isPlaying != prev.isPlaying) {
    drawButtons(cur.isPlaying);
  }

  // Calculate live elapsed position
  uint32_t elapsed = cur.positionMs;
  if (cur.isPlaying && cur.lastPosUpdateMs > 0) {
    elapsed += (millis() - cur.lastPosUpdateMs);
  }
  if (cur.durationMs > 0 && elapsed > cur.durationMs) {
    elapsed = cur.durationMs;
  }

  // Redraw scrubber when the second counter changes or when duration/play status changes
  static uint32_t s_lastDrawnSec = 999999;
  uint32_t curSec = elapsed / 1000;
  if (curSec != s_lastDrawnSec || cur.durationMs != prev.durationMs || cur.isPlaying != prev.isPlaying) {
    drawScrubber(elapsed, cur.durationMs);
    s_lastDrawnSec = curSec;
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

  // Uppercase for stylish funky look
  for (int i = 0; i < a.length(); i++) a[i] = toupper(a[i]);

  display.setTextDatum(lgfx::top_left);
  display.setTextColor(c(display, COL_TEXT_MAIN));

  const int availableWidth = 272; // 320 - 24*2

  // Start with Font4 (bold display typography)
  display.setFont(&fonts::Font4);
  display.setTextSize(1.0f);
  int w = display.textWidth(a.c_str());

  if (w <= availableWidth) {
    display.drawString(a.c_str(), 24, 30);
  } else {
    float scale = (float)availableWidth / (float)w;
    if (scale >= 0.60f) {
      display.setTextSize(scale);
      display.drawString(a.c_str(), 24, 30);
      display.setTextSize(1.0f);
    } else {
      display.setFont(&fonts::Font2);
      display.setTextSize(1.0f);
      int w2 = display.textWidth(a.c_str());
      float scale2 = (w2 > availableWidth) ? ((float)availableWidth / (float)w2) : 1.0f;
      display.setTextSize(scale2);
      display.drawString(a.c_str(), 24, 34);
      display.setTextSize(1.0f);
    }
  }
}

// ----------------------------------------------------------------
void MusicScreen::drawTitle(const String& title) {
  String t = title.isEmpty() ? "Undenied" : title;

  display.setTextDatum(lgfx::top_left);
  display.setFont(&fonts::Font2);
  display.setTextColor(c(display, COL_TEXT_SUB));

  const int availableWidth = 272; // 320 - 24*2
  display.setTextSize(1.0f);
  int w = display.textWidth(t.c_str());

  if (w <= availableWidth) {
    display.drawString(t.c_str(), 24, 68);
  } else {
    float scale = (float)availableWidth / (float)w;
    if (scale >= 0.72f) {
      display.setTextSize(scale);
      display.drawString(t.c_str(), 24, 68);
      display.setTextSize(1.0f);
    } else {
      int mid = t.length() / 2;
      int splitPos = -1;
      int minDiff = 999;
      for (int i = 0; i < t.length(); i++) {
        if (t[i] == ' ') {
          int d = abs(i - mid);
          if (d < minDiff) {
            minDiff = d;
            splitPos = i;
          }
        }
      }

      if (splitPos != -1) {
        String line1 = t.substring(0, splitPos);
        String line2 = t.substring(splitPos + 1);
        int w1 = display.textWidth(line1.c_str());
        int w2 = display.textWidth(line2.c_str());
        int maxW = max(w1, w2);
        float lineScale = 1.0f;
        if (maxW > availableWidth) {
          lineScale = (float)availableWidth / (float)maxW;
        }
        display.setTextSize(lineScale);
        display.drawString(line1.c_str(), 24, 60);
        display.drawString(line2.c_str(), 24, 78);
        display.setTextSize(1.0f);
      } else {
        display.setTextSize(scale);
        display.drawString(t.c_str(), 24, 68);
        display.setTextSize(1.0f);
      }
    }
  }
}

// ----------------------------------------------------------------
void MusicScreen::drawScrubber(uint32_t elapsedMs, uint32_t durationMs) {
  uint16_t bg = c(display, COL_BG);

  // Clear scrubber line and timestamps zone (y: 120..146)
  display.fillRect(20, 120, 280, 27, bg);

  const int barX = 24;
  const int barY = 124;
  const int barW = 272;

  // Background hairline track
  display.drawFastHLine(barX, barY, barW, c(display, COL_LINE_BG));

  int activeW = 0;
  if (durationMs > 0) {
    float frac = (float)elapsedMs / (float)durationMs;
    if (frac > 1.0f) frac = 1.0f;
    if (frac < 0.0f) frac = 0.0f;
    activeW = (int)(frac * (float)barW);
  }

  // Active track fill & scrubber knob
  if (activeW > 0) {
    display.drawFastHLine(barX, barY, activeW, c(display, COL_LINE_ACTIVE));
    display.fillCircle(barX + activeW, barY, 3, c(display, COL_LINE_ACTIVE));
  } else {
    display.fillCircle(barX, barY, 3, c(display, COL_LINE_ACTIVE));
  }

  // Timestamps
  display.setFont(&fonts::Font0); // 8-pixel minimal font
  display.setTextColor(c(display, COL_TEXT_SUB));

  // Elapsed time (MM:SS)
  uint32_t elSec = elapsedMs / 1000;
  char elBuf[16];
  snprintf(elBuf, sizeof(elBuf), "%u:%02u", elSec / 60, elSec % 60);
  display.setTextDatum(lgfx::top_left);
  display.drawString(elBuf, barX, 132);

  // Total duration (MM:SS)
  if (durationMs > 0) {
    uint32_t durSec = durationMs / 1000;
    char durBuf[16];
    snprintf(durBuf, sizeof(durBuf), "%u:%02u", durSec / 60, durSec % 60);
    display.setTextDatum(lgfx::top_right);
    display.drawString(durBuf, barX + barW, 132);
  }
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
