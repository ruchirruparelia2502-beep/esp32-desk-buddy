#pragma once

// ============================================================
//  MusicScreen.h — Minimalist Funky Music Screen Declaration
// ============================================================

#include "../AppState.h"
#include "DisplaySetup.h"

class MusicScreen {
public:
  /**
   * Full initial draw of the music screen.
   */
  static void draw();

  /**
   * Lightweight update — repaints only changed text or play state.
   */
  static void update(const TrackInfo& prev);

  /**
   * Handle touch input across the control buttons.
   */
  static bool handleTouch(int tx, int ty);

private:
  static void drawBackground();
  static void drawArtist(const String& artist);
  static void drawTitle(const String& title);
  static void drawScrubber();
  static void drawButtons(bool isPlaying);
};
