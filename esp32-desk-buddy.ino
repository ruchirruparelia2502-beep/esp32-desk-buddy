// ============================================================
//  main.cpp — ESP32 Desk Buddy
//  Board: ESP32-2432S024C (CYD 2.4" Capacitive)
//
//  Responsibilities:
//   - WiFi connect → NTP → Weather
//   - BLE (iOS AMS) + Classic BT A2DP (Android) start
//   - Main loop: screen state machine, touch polling, dirty redraw
// ============================================================

#include <Arduino.h>
#include <WiFi.h>

// ---- Display ----
#include "DisplaySetup.h"
LGFX display;   // global instance referenced by all UI modules

// ---- App state ----
#include "AppState.h"
AppState g_state;  // global definition

// ---- Modules ----
#include "config.h"
#include "IdleScreen.h"
#include "MusicScreen.h"
#include "TouchManager.h"
#include "AMSClient.h"
#include "AVRCPHandler.h"
#include "TimeSync.h"
// ================================================================
//  Forward declarations
// ================================================================
static void connectWiFi();
static void startNetworkServices();

// ================================================================
//  Snapshot of previous state for dirty-flag comparison
// ================================================================
static Screen   s_lastScreen     = Screen::IDLE;
static TrackInfo s_lastTrack;
static String   s_lastTime       = "";
static String   s_lastDate       = "";
static bool     s_fullRedrawPending = true;

// ================================================================
//  setup()
// ================================================================
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n[Boot] ESP32 Desk Buddy starting...");

  // ---- Mutex ----
  g_state.mutex = xSemaphoreCreateMutex();
  configASSERT(g_state.mutex);

  // ---- Display init ----
  display.init();
  display.setRotation(1);  // landscape
  display.setBrightness(220);
  display.fillScreen(TFT_BLACK);
  Serial.println("[Boot] Display initialised");

  // ---- Boot splash ----
  display.setTextDatum(lgfx::middle_center);
  display.setFont(&fonts::Font4);
  display.setTextColor(TFT_WHITE);
  display.drawString("Desk Buddy", 160, 100);
  display.setFont(&fonts::Font2);
  display.setTextColor(display.color888(0x88, 0x88, 0x88));
  display.drawString("Starting up...", 160, 130);

  // ---- Touch init ----
  if (!TouchManager::begin()) {
    Serial.println("[Boot] Touch init failed (continuing anyway)");
  }

  // ---- WiFi ----
  connectWiFi();

  // ---- Network services (NTP) ----
  if (g_state.wifiConnected) {
    startNetworkServices();
  }

  // ---- Bluetooth ----
  // NOTE: BLE (NimBLE for iOS AMS) and Classic BT (A2DP for Android)
  // share the same radio but use different stacks. ESP32-A2DP initialises
  // Bluedroid; NimBLE uses its own stack. They CAN coexist if BT/BLE
  // dual mode is active, but simultaneous connection is not guaranteed.
  // Practical use: pair one phone at a time.

  Serial.println("[Boot] Starting BLE (iOS AMS)...");
  AMSClient::begin();

  Serial.println("[Boot] Starting Classic BT A2DP (Android)...");
  AVRCPHandler::begin();

  // ---- First draw ----
  s_fullRedrawPending = true;

  Serial.println("[Boot] Setup complete");
}

// ================================================================
//  loop()
// ================================================================
void loop() {
  // ---- Time tick ----
  bool timeChanged = TimeSync::tick();

  // ---- Determine target screen ----
  Screen targetScreen;
  TrackInfo curTrack;
  String    curTime, curDate;

  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    curTrack   = g_state.track;
    curTime    = g_state.timeStr;
    curDate    = g_state.dateStr;

    bool musicActive =
      !g_state.track.title.isEmpty() &&
      (millis() - g_state.lastMusicUpdateMs) < MUSIC_IDLE_TIMEOUT_MS;

    targetScreen = musicActive ? Screen::MUSIC : Screen::IDLE;
    g_state.currentScreen = targetScreen;

    xSemaphoreGive(g_state.mutex);
  } else {
    targetScreen = s_lastScreen;
    curTrack     = s_lastTrack;
    curTime      = s_lastTime;
    curDate      = s_lastDate;
  }

  // ---- Screen state machine ----
  bool screenChanged = (targetScreen != s_lastScreen);

  if (screenChanged || s_fullRedrawPending) {
    s_fullRedrawPending = false;
    s_lastScreen        = targetScreen;

    if (targetScreen == Screen::IDLE) {
      Serial.println("[UI] → Idle screen");
      IdleScreen::draw();
    } else {
      Serial.println("[UI] → Music screen");
      MusicScreen::draw();
    }

    // Reset comparison snapshots after full redraw
    s_lastTrack = curTrack;
    s_lastTime  = curTime;
    s_lastDate  = curDate;

  } else {
    // ---- Incremental updates ----
    if (targetScreen == Screen::IDLE) {
      IdleScreen::update(s_lastTime, s_lastDate);
      s_lastTime = curTime;
      s_lastDate = curDate;
    } else {
      MusicScreen::update(s_lastTrack);
      s_lastTrack = curTrack;
    }
  }

  // ---- Touch handling ----
  TouchPoint tp;
  if (TouchManager::isTapped(tp)) {
    Serial.printf("[Touch] Tap at (%d, %d)\n", tp.x, tp.y);

    if (targetScreen == Screen::MUSIC) {
      if (MusicScreen::handleTouch(tp.x, tp.y)) {
        // Brief visual feedback: re-draw buttons after a tick
        delay(80);
        MusicScreen::update(s_lastTrack);  // refresh play/pause button state
      }
    }
  }

  // ---- Housekeeping ----
  delay(50);  // ~20 Hz loop — sufficient for a clock + music UI
}

// ================================================================
//  connectWiFi()
// ================================================================
static void connectWiFi() {
  Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);

  // Show connecting message on display
  display.fillScreen(display.color888(0x0A, 0x0A, 0x1A));
  display.setTextDatum(lgfx::middle_center);
  display.setFont(&fonts::Font4);
  display.setTextColor(TFT_WHITE);
  display.drawString("Connecting to WiFi", 160, 100);
  display.setFont(&fonts::Font2);
  display.setTextColor(display.color888(0x88, 0x88, 0x88));
  display.drawString(WIFI_SSID, 160, 130);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long deadline = millis() + 15000UL;  // 15 s timeout
  while (WiFi.status() != WL_CONNECTED && millis() < deadline) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n[WiFi] Connected! IP: %s\n",
                  WiFi.localIP().toString().c_str());

    if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      g_state.wifiConnected = true;
      xSemaphoreGive(g_state.mutex);
    }

    display.fillRect(0, 115, 320, 30,
      display.color888(0x0A, 0x0A, 0x1A));
    display.setTextDatum(lgfx::middle_center);
    display.setFont(&fonts::Font2);
    display.setTextColor(display.color888(0x1D, 0xB9, 0x54));
    display.drawString("Connected!", 160, 130);
    delay(800);

  } else {
    Serial.println("\n[WiFi] Connection failed — continuing offline");
    display.fillRect(0, 115, 320, 30,
      display.color888(0x0A, 0x0A, 0x1A));
    display.setFont(&fonts::Font2);
    display.setTextColor(TFT_RED);
    display.setTextDatum(lgfx::middle_center);
    display.drawString("WiFi failed (offline mode)", 160, 130);
    delay(1500);
  }
}

// ================================================================
//  startNetworkServices()
// ================================================================
static void startNetworkServices() {
  TimeSync::begin();
}
