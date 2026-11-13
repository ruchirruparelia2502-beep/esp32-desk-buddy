// ============================================================
//  AVRCPHandler.cpp — Android & iOS Classic BT A2DP Sink + AVRCP
// ============================================================

#include "AVRCPHandler.h"
#include <BluetoothA2DPSink.h>

// Global A2DP sink instance
static BluetoothA2DPSink a2dp_sink;

// Dummy stream callback to consume PCM audio data without initializing I2S.
static void _dummyAudioStream(const uint8_t *data, uint32_t length) {
  // Discard raw PCM audio samples — Desk Buddy is a display-only device
}

// ----------------------------------------------------------------
void AVRCPHandler::begin() {
  // Disable I2S hardware output (false = do NOT initialize I2S peripheral or pins)
  a2dp_sink.set_stream_reader(_dummyAudioStream, false);

  // Register AVRCP metadata callback including duration (PLAYING_TIME)
  a2dp_sink.set_avrc_metadata_attribute_mask(
    ESP_AVRC_MD_ATTR_TITLE   |
    ESP_AVRC_MD_ATTR_ARTIST  |
    ESP_AVRC_MD_ATTR_ALBUM   |
    ESP_AVRC_MD_ATTR_PLAYING_TIME
  );
  a2dp_sink.set_avrc_metadata_callback(_avrcMetadataCB);
  a2dp_sink.set_avrc_rn_playstatus_callback(_avrcPlayStatusCB);
  a2dp_sink.set_avrc_rn_play_pos_callback(_avrcPlayPosCB, 1); // request pos updates every 1 sec
  a2dp_sink.set_on_connection_state_changed(_connectionStateCB, nullptr);

  // Start A2DP sink
  a2dp_sink.start(A2DP_DEVICE_NAME);

  Serial.printf("[A2DP] Universal Bluetooth started: %s\n", A2DP_DEVICE_NAME);
}

// ----------------------------------------------------------------
void AVRCPHandler::_avrcMetadataCB(uint8_t id, const uint8_t* text) {
  if (!text) return;
  String val = String((const char*)text);

  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) != pdTRUE) return;

  switch (id) {
    case ESP_AVRC_MD_ATTR_TITLE:
      g_state.track.title = val;
      break;
    case ESP_AVRC_MD_ATTR_ARTIST:
      g_state.track.artist = val;
      break;
    case ESP_AVRC_MD_ATTR_ALBUM:
      g_state.track.album = val;
      break;
    case ESP_AVRC_MD_ATTR_PLAYING_TIME:
      g_state.track.durationMs = val.toInt();
      Serial.printf("[AVRCP] Track duration: %u ms (%u sec)\n",
                    g_state.track.durationMs, g_state.track.durationMs / 1000);
      break;
    default:
      break;
  }
  g_state.lastMusicUpdateMs = millis();

  xSemaphoreGive(g_state.mutex);

  Serial.printf("[AVRCP] Meta id=%d val='%s'\n", id, val.c_str());
}

// ----------------------------------------------------------------
void AVRCPHandler::_avrcPlayStatusCB(esp_avrc_playback_stat_t playback) {
  bool playing = (playback == ESP_AVRC_PLAYBACK_PLAYING);

  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    g_state.track.isPlaying = playing;
    if (playing) {
      g_state.lastMusicUpdateMs = millis();
      g_state.track.lastPosUpdateMs = millis();
    }
    xSemaphoreGive(g_state.mutex);
  }

  Serial.printf("[AVRCP] Play status: %s\n",
                playing ? "Playing" : "Paused/Stopped");
}

// ----------------------------------------------------------------
void AVRCPHandler::_avrcPlayPosCB(uint32_t play_pos) {
  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    g_state.track.positionMs = play_pos;
    g_state.track.lastPosUpdateMs = millis();
    g_state.lastMusicUpdateMs = millis();
    xSemaphoreGive(g_state.mutex);
  }
}

// ----------------------------------------------------------------
void AVRCPHandler::_connectionStateCB(esp_a2d_connection_state_t state,
                                       void* ptr) {
  if (state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
    Serial.println("[A2DP] Bluetooth device connected!");
    if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      g_state.btSource = BTSource::ANDROID_BT;
      xSemaphoreGive(g_state.mutex);
    }
  } else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
    Serial.println("[A2DP] Bluetooth device disconnected");
    if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      g_state.btSource       = BTSource::NONE;
      g_state.track          = TrackInfo{};
      g_state.currentScreen  = Screen::IDLE;
      xSemaphoreGive(g_state.mutex);
    }
  }
}

// ----------------------------------------------------------------
void AVRCPHandler::play() {
  Serial.println("[AVRCP] → Play");
  a2dp_sink.play();
}

void AVRCPHandler::pause() {
  Serial.println("[AVRCP] → Pause");
  a2dp_sink.pause();
}

void AVRCPHandler::togglePlayPause() {
  bool cur = false;
  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(20)) == pdTRUE) {
    cur = g_state.track.isPlaying;
    xSemaphoreGive(g_state.mutex);
  }
  if (cur) {
    pause();
  } else {
    play();
  }
}

void AVRCPHandler::next() {
  Serial.println("[AVRCP] → Next");
  a2dp_sink.next();
}

void AVRCPHandler::prev() {
  Serial.println("[AVRCP] → Previous");
  a2dp_sink.previous();
}
