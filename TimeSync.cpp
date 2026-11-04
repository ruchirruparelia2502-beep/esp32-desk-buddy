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
void TimeSync::_updateStrings() {
  if (!s_ntp.isTimeSet()) {
    if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      g_state.timeStr = "--:--";
      g_state.dateStr = "---";
      xSemaphoreGive(g_state.mutex);
    }
    return;
  }

  // Build "HH:MM"
  char timeBuf[8];
  snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d",
           s_ntp.getHours(), s_ntp.getMinutes());

  // Build "Mon, 04 Nov"
  // NTPClient gives day of week (0=Sunday) and epoch
  unsigned long epoch   = s_ntp.getEpochTime();
  int dayOfWeek = s_ntp.getDay();  // 0=Sun

  // Compute day, month from epoch
  unsigned long days    = epoch / 86400UL;
  // Offset: epoch day 0 = Thu 1 Jan 1970
  // Zeller-like simple decomposition
  unsigned long rem     = epoch % 86400UL;
  (void)rem;

  // Use mktime/gmtime approach via struct tm-compatible math
  // Simple: use epoch seconds
  time_t t = (time_t)epoch;
  struct tm* gmt = gmtime(&t);
  int day   = gmt->tm_mday;
  int month = gmt->tm_mon + 1;   // 1..12

  char dateBuf[20];
  snprintf(dateBuf, sizeof(dateBuf), "%s, %02d %s",
           DAY_NAMES[dayOfWeek], day, MONTH_NAMES[month]);

  if (xSemaphoreTake(g_state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    g_state.timeStr = String(timeBuf);
    g_state.dateStr = String(dateBuf);
    xSemaphoreGive(g_state.mutex);
  }
}
