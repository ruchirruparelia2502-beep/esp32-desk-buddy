#pragma once

// ============================================================
//  IdleScreen.h — Idle (clock) screen declaration
// ============================================================

#include "AppState.h"
#include "DisplaySetup.h"

class IdleScreen {
public:
  /**
   * Full initial draw of the idle screen.
   */
  static void draw();

  /**
   * Lightweight update — repaints time/date if changed.
   */
  static void update(const String& prevTime, const String& prevDate);

  /**
   * Handle touch tap on idle screen to toggle 12h/24h mode.
   */
  static bool handleTouch(int tx, int ty);

private:
  static void drawBackground();
  static void drawTime(const String& t, const String& ampm);
  static void drawDate(const String& d);
  static void drawDivider();
  static void drawFooter();
};
