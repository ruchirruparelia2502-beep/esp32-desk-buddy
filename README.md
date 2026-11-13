# ESP32 Desk Buddy — ESP32-2432S024C (CYD 2.4" Capacitive)

A minimalist monochrome firmware for the Cheap Yellow Display (CYD) 2.4" Capacitive Touch board.

- 🎵 **Music Now Playing** — iOS via BLE Apple Media Service (AMS), Android via Classic BT AVRCP
- ⏰ **NTP Clock** — Automatically syncs accurate time over Wi-Fi
- 🖤 **Minimalist OLED Dark UI** — Unified deep black aesthetic with dynamic auto-fit typography
- 👆 **Touch Controls** — Skip Back (`|<`), Play/Pause circular button, Skip Forward (`>|`)
- 🔋 **Battery Support** — 3.7V LiPo with onboard charging via 2-pin JST 1.25mm connector

---

## 🚀 Arduino IDE Setup Guide

### 1. Install ESP32 Board Support in Arduino IDE
1. Open **Arduino IDE**.
2. Go to **File → Preferences** (or `Cmd + ,` on macOS).
3. In **Additional Board Manager URLs**, add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager...**, search for `esp32` by **Espressif Systems**, and click **Install**.

---

### 2. Install Required Libraries
Open **Tools → Manage Libraries...** (or `Cmd + Shift + I` on macOS) and install:

| Library Name | Author | Version | Notes |
|--------------|--------|---------|-------|
| **LovyanGFX** | lovyan03 | Latest (1.1.x+) | High-performance graphics driver |
| **NTPClient** | Fabrice Weinberg / arduino-libraries | Latest (3.2.x+) | NTP time synchronization |
| **NimBLE-Arduino** | h2zero | Latest (1.4.x+) | Fast, lightweight BLE stack for iOS AMS |
| **ESP32-A2DP** | Phil Schatzmann | Latest (1.8.x+) | Android Bluetooth A2DP/AVRCP sink |

> 💡 *Note: If ESP32-A2DP asks to install dependencies, allow it.*

---

### 3. Open the Project in Arduino IDE
1. Download or clone this repository:
   ```bash
   git clone https://github.com/ruchirruparelia2502-beep/esp32-desk-buddy.git
   ```
2. In Arduino IDE, go to **File → Open...** and select **`esp32-desk-buddy.ino`**.
3. All code files will open as tabs across the top!
4. Click the **`config.h`** tab and enter your Wi-Fi details:
   ```cpp
   #define WIFI_SSID       "YourWiFiNetwork"
   #define WIFI_PASSWORD   "YourWiFiPassword"
   #define NTP_UTC_OFFSET  19800   // Seconds offset (e.g. UTC+5:30 IST = 19800)
   #define TIME_FORMAT_24H true    // true = 24-hour (14:30), false = 12-hour (02:30 PM)
   ```

   > 💡 *Tip: You can also tap anywhere on the screen during clock mode to toggle between 12-Hour and 24-Hour on the fly!*

---

### 4. Board & Tools Settings (Important!)
Configure your **Tools** menu in Arduino IDE:

- **Board:** `ESP32 Dev Module`
- **CPU Frequency:** `240MHz (WiFi/BT)`
- **Flash Frequency:** `80MHz`
- **Flash Mode:** `QIO`
- **Flash Size:** `4MB (32Mb)`
- **Partition Scheme:** `Huge APP (3MB No OTA/1MB SPIFFS)` ⚠️ *(Crucial so Bluetooth fits)*
- **Core Debug Level:** `None`
- **Port:** Select your board's serial port (e.g. `/dev/cu.usbserial-...` or `COMx`)

---

### 5. Upload!
Click the **Upload** button (arrow icon `➜`) in Arduino IDE.

---

## 📱 Pairing Instructions

### iOS (Apple Music, Spotify, Podcasts, YouTube Music)
1. Open **Settings → Bluetooth** on your iPhone.
2. Select and pair with **"DeskBuddy"**.
3. Start playing music — track title and artist appear automatically via Apple Media Service (AMS).

### Android
1. Open **Settings → Bluetooth** on your Android device.
2. Pair with **"DeskBuddy BT"**.
3. Play music — metadata appears automatically via AVRCP.

---

## 📌 Hardware Pinout (ESP32-2432S024C)

| Function | Pin | Notes |
|----------|-----|-------|
| LCD MOSI | GPIO 13 | SPI Data Out |
| LCD MISO | GPIO 12 | SPI Data In |
| LCD SCLK | GPIO 14 | SPI Clock |
| LCD CS | GPIO 15 | Chip Select |
| LCD DC | GPIO 2 | Data / Command |
| LCD Backlight | GPIO 21 | Backlight PWM |
| Touch SDA | GPIO 33 | I2C Data |
| Touch SCL | GPIO 32 | I2C Clock |
| Touch INT | GPIO 36 | Interrupt |
| Touch RST | GPIO 25 | Reset |

---

## 📜 License & Commercial Use Notice

This project is licensed under the **Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License (CC BY-NC-SA 4.0)**.

- ❌ **Non-Commercial**: You may **NOT** use this project, its code, hardware designs, or derivatives for commercial purposes. You cannot manufacture, sell, or monetize devices running this firmware without explicit written permission.
- ✅ **Personal / Non-Commercial Use**: You are free to build, modify, and run this for personal DIY projects.
- 🔄 **ShareAlike**: Any modifications or derivatives must be shared under this same license.

See the [LICENSE](LICENSE) file for complete details.
