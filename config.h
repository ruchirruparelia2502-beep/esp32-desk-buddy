#pragma once

// ============================================================
//  config.h — User-configurable credentials and constants
// ============================================================

// ----- WiFi -----
#define WIFI_SSID       "YourSSID"
#define WIFI_PASSWORD   "YourPassword"

// ----- NTP & Time Format -----
#define NTP_SERVER      "pool.ntp.org"
#define NTP_UTC_OFFSET  19800   // seconds east of UTC (e.g. IST = 5.5h = 19800)
#define NTP_UPDATE_INTERVAL_MS  3600000UL  // 1 hour
#define TIME_FORMAT_24H true    // true = 24-hour (14:30), false = 12-hour (02:30 PM)

// ----- Display pins (ILI9341 SPI) -----
#define PIN_LCD_MOSI    13
#define PIN_LCD_MISO    12
#define PIN_LCD_SCLK    14
#define PIN_LCD_CS      15
#define PIN_LCD_DC       2
#define PIN_LCD_RST     -1   // tied to 3V3 on board
#define PIN_LCD_BL      21

// ----- Touch pins (CST820 I2C) -----
#define PIN_TOUCH_SDA   33
#define PIN_TOUCH_SCL   32
#define PIN_TOUCH_INT   36
#define PIN_TOUCH_RST   25

// ----- BLE (NimBLE / AMS) -----
#define BLE_DEVICE_NAME "DeskBuddy"

// AMS Service UUID
#define AMS_SERVICE_UUID         "89D3502B-0F36-433A-8EF4-C502AD55F8DC"
// AMS Characteristic UUIDs
#define AMS_REMOTE_CMD_UUID      "9B3C81D8-57B1-4A8A-B8DF-0E56F7CA51C3"
#define AMS_ENTITY_UPDATE_UUID   "2F7CABCE-808D-411F-9A0C-BB92BA96C102"
#define AMS_ENTITY_ATTR_UUID     "C6B2F38C-23AB-46D8-A6AB-A3A870BBD5D7"

// AMS Entity IDs
#define AMS_ENTITY_PLAYER  0x00
#define AMS_ENTITY_QUEUE   0x01
#define AMS_ENTITY_TRACK   0x02

// AMS Track Attribute IDs
#define AMS_TRACK_ATTR_ARTIST   0x00
#define AMS_TRACK_ATTR_ALBUM    0x01
#define AMS_TRACK_ATTR_TITLE    0x02
#define AMS_TRACK_ATTR_DURATION 0x03

// AMS Remote Commands
#define AMS_CMD_PLAY           0x00
#define AMS_CMD_PAUSE          0x01
#define AMS_CMD_TOGGLE_PLAY    0x02
#define AMS_CMD_NEXT           0x03
#define AMS_CMD_PREV           0x04

// AMS Player Attribute IDs
#define AMS_PLAYER_ATTR_NAME        0x00
#define AMS_PLAYER_ATTR_PLAY_INFO   0x01  // "Playing"|"Paused"|"Stopped"
#define AMS_PLAYER_ATTR_VOLUME      0x02

// ----- Classic Bluetooth / A2DP -----
#define A2DP_DEVICE_NAME "DeskBuddy BT"

// ----- Music activity timeout -----
#define MUSIC_IDLE_TIMEOUT_MS  30000UL  // switch back to idle after 30s of no update

// ----- Touch debounce -----
#define TOUCH_DEBOUNCE_MS  300

// ----- Music screen button layout (landscape 320x240) -----
#define BTN_PREV_X   10
#define BTN_PREV_W   85
#define BTN_PLAY_X   115
#define BTN_PLAY_W   90
#define BTN_NEXT_X   225
#define BTN_NEXT_W   85
#define BTN_Y        155
#define BTN_H        70
#define BTN_AREA_Y   150   // only process touch below this Y
