// ============================================================
//  TimeSync.cpp — NTP time synchronisation via NTPClient
// ============================================================

#include "TimeSync.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "config.h"
#include "AppState.h"

static WiFiUDP    s_udp;
static NTPClient  s_ntp(s_udp, NTP_SERVER, NTP_UTC_OFFSET, NTP_UPDATE_INTERVAL_MS);

bool          TimeSync::_started      = false;
unsigned long TimeSync::_lastUpdateMs = 0;

static const char* DAY_NAMES[] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
static const char* MONTH_NAMES[] = {
  "", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

// ----------------------------------------------------------------
void TimeSync::begin() {
  s_ntp.begin();
  s_ntp.forceUpdate();
  _started = true;
  _updateStrings();
  Serial.println("[NTP] Time sync started");
}

// ----------------------------------------------------------------
bool TimeSync::tick() {
  if (!_started) return false;

  unsigned long now = millis();
  // Update NTP library at the library's own interval
  s_ntp.update();

  // Update display strings every 30 seconds
  if ((now - _lastUpdateMs) >= 30000UL || _lastUpdateMs == 0) {
    _lastUpdateMs = now;
    _updateStrings();
    return true;
  }
  return false;
}

// ----------------------------------------------------------------
void TimeSync::forceRefresh() {
  _lastUpdateMs = millis();
  _updateStrings();
}

// ----------------------------------------------------------------
void TimeSync::_updateStrings() {
  if (!s_ntp.isTimeSet()) {
    if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      g_state.timeStr = "--:--";
      g_state.ampmStr = "";
      g_state.dateStr = "---";
      xSemaphoreGive(g_state.mutex);
    }
    return;
  }

  int rawHours = s_ntp.getHours();
  int minutes  = s_ntp.getMinutes();
  int displayHours = rawHours;
  char ampmBuf[4] = "";

  bool is24h = true;
  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    is24h = g_state.is24Hour;
    xSemaphoreGive(g_state.mutex);
  }

  // Format 12-hour vs 24-hour
  if (!is24h) {
    if (rawHours == 0) {
      displayHours = 12;
      strcpy(ampmBuf, "AM");
    } else if (rawHours == 12) {
      displayHours = 12;
      strcpy(ampmBuf, "PM");
    } else if (rawHours > 12) {
      displayHours = rawHours - 12;
      strcpy(ampmBuf, "PM");
    } else {
      strcpy(ampmBuf, "AM");
    }
  }

  // Build time string
  char timeBuf[12];
  snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", displayHours, minutes);

  // Build "Mon, 04 Nov"
  unsigned long epoch = s_ntp.getEpochTime();
  int dayOfWeek = s_ntp.getDay();  // 0=Sun

  time_t t = (time_t)epoch;
  struct tm* gmt = gmtime(&t);
  int day   = gmt->tm_mday;
  int month = gmt->tm_mon + 1;   // 1..12

  char dateBuf[24];
  snprintf(dateBuf, sizeof(dateBuf), "%s, %02d %s",
           DAY_NAMES[dayOfWeek], day, MONTH_NAMES[month]);

  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    g_state.timeStr = String(timeBuf);
    g_state.ampmStr = String(ampmBuf);
    g_state.dateStr = String(dateBuf);
    xSemaphoreGive(g_state.mutex);
  }
}
