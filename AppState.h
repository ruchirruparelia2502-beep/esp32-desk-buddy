#pragma once
#include <Arduino.h>

// ============================================================
//  AppState.h — Global shared state between all modules
// ============================================================

enum class Screen   { IDLE, MUSIC };
enum class BTSource { NONE, IOS_BLE, ANDROID_BT };

struct TrackInfo {
  String title;
  String artist;
  String album;
  bool   isPlaying = false;
};

struct AppState {
  Screen   currentScreen      = Screen::IDLE;
  BTSource btSource           = BTSource::NONE;
  TrackInfo  track;
  String   timeStr;              // "14:05" or "02:05"
  String   ampmStr;              // "AM", "PM", or "" (in 24h mode)
  String   dateStr;              // "Mon, 04 Nov"
  bool     is24Hour           = true;
  unsigned long lastMusicUpdateMs = 0;
  bool     wifiConnected      = false;
  SemaphoreHandle_t mutex     = nullptr;
};

extern AppState g_state;
