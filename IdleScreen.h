#pragma once

// ============================================================
//  IdleScreen.h — Idle (clock + weather) screen declaration
// ============================================================

#include "AppState.h"
#include "DisplaySetup.h"

class IdleScreen {
public:
  /**
   * Full initial draw of the idle screen. Call when switching
   * to this screen from another, or on first boot.
   */
  static void draw();

  /**
   * Lightweight update — only repaints regions that have changed.
   * Call from the main loop every second.
   * @param prevTime  Previous time string (to detect changes)
   * @param prevDate  Previous date string
   */
  static void update(const String& prevTime, const String& prevDate);

private:
  static void drawBackground();
  static void drawTime(const String& t);
  static void drawDate(const String& d);
  static void drawDivider();
  static void drawFooter();
};
