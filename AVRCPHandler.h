#pragma once

// ============================================================
//  AVRCPHandler.h — Android A2DP Sink + AVRCP metadata handler
// ============================================================

#include <Arduino.h>
#include <esp_avrc_api.h>
#include <esp_a2dp_api.h>
#include "AppState.h"
#include "config.h"

class AVRCPHandler {
public:
  /**
   * Initialise the A2DP Sink (BluetoothA2DSink).
   * Call once from setup() — the BT stack runs asynchronously.
   */
  static void begin();

  // ---- Playback controls (write AVRCP passthrough commands) ----
  static void play();
  static void pause();
  static void togglePlayPause();
  static void next();
  static void prev();

private:
  // A2DP callbacks (static so they can be passed as C function pointers)
  static void _avrcMetadataCB(uint8_t id, const uint8_t* text);
  static void _avrcPlayStatusCB(esp_avrc_playback_stat_t playback);
  static void _avrcPlayPosCB(uint32_t play_pos);
  static void _connectionStateCB(esp_a2d_connection_state_t state,
                                  void* ptr);
};
