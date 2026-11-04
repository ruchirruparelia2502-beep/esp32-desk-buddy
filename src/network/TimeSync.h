#pragma once

// ============================================================
//  TimeSync.h — NTP time synchronisation
// ============================================================

#include <Arduino.h>

class TimeSync {
public:
  /**
   * Begin NTP client. Call after WiFi is connected.
   * Updates g_state.timeStr and g_state.dateStr on each tick.
   */
  static void begin();

  /**
   * Call from the main loop. Updates time strings in g_state
   * if the minute has changed. Rate-limited internally.
   * @return true if the time string was updated
   */
  static bool tick();

private:
  static bool _started;
  static unsigned long _lastUpdateMs;
  static void _updateStrings();
};
