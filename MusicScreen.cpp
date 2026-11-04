// ============================================================
//  MusicScreen.cpp — Minimalist Funky OLED Black Music UI
//  Auto-fitting typography: Entire title & artist ALWAYS fit!
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

  // If artist or title changes, clear the combined meta area
  if (cur.artist != prev.artist || cur.title != prev.title) {
    display.fillRect(20, 24, 280, 88, bg);
    drawArtist(cur.artist);
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

  // Uppercase for stylish funky look
  for (int i = 0; i < a.length(); i++) a[i] = toupper(a[i]);

  display.setTextDatum(lgfx::top_left);
  display.setFont(&fonts::Font4);
  display.setTextColor(c(display, COL_TEXT_MAIN));

  const int availableWidth = 272; // 320 - 24*2
  display.setTextSize(1.0f);
  int w = display.textWidth(a.c_str());

  if (w <= availableWidth) {
    display.drawString(a.c_str(), 24, 30);
  } else {
    // Dynamically scale down so full artist fits
    float scale = (float)availableWidth / (float)w;
    display.setTextSize(scale);
    display.drawString(a.c_str(), 24, 30);
    display.setTextSize(1.0f);
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
    // Fits comfortably in 1 line
    display.drawString(t.c_str(), 24, 68);
  } else {
    // Check if slight proportional scaling fits 1 line
    float scale = (float)availableWidth / (float)w;
    if (scale >= 0.72f) {
      display.setTextSize(scale);
      display.drawString(t.c_str(), 24, 68);
      display.setTextSize(1.0f);
    } else {
      // Split into 2 clean lines so entire title is 100% visible
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
        // No space found, scale down directly
        display.setTextSize(scale);
        display.drawString(t.c_str(), 24, 68);
        display.setTextSize(1.0f);
      }
    }
  }
}

// ----------------------------------------------------------------
void MusicScreen::drawScrubber() {
  // Ultra-minimal hairline progress track
  display.drawFastHLine(24, 126, 272, c(display, COL_LINE_BG));
  display.drawFastHLine(24, 126, 105, c(display, COL_LINE_ACTIVE));
  display.fillCircle(129, 126, 3, c(display, COL_LINE_ACTIVE));
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
