#pragma once

// ============================================================
//  MusicScreen.h — Music (now-playing) screen declaration
// ============================================================

#include "../AppState.h"
#include "DisplaySetup.h"

class MusicScreen {
public:
  /**
   * Full initial draw of the music screen. Call when switching
   * to this screen from another.
   */
  static void draw();

  /**
   * Lightweight update — only repaints regions that changed.
   * Pass the previous track snapshot to detect diffs.
   */
  static void update(const TrackInfo& prev);

  /**
   * Handle a touch event at (tx, ty).  Returns true if a BT command
   * was sent as a result.
   */
  static bool handleTouch(int tx, int ty);

private:
  static void drawBackground();
  static void drawHeader(BTSource src);
  static void drawTitle(const String& title);
  static void drawArtist(const String& artist);
  static void drawAlbum(const String& album);
  static void drawDivider();
  static void drawButtons(bool isPlaying);
  static void drawButton(int x, int y, int w, int h,
                         uint32_t bgColor, const char* label);
};
