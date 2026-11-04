# ESP32 Desk Buddy — ESP32-2432S024C (CYD 2.4" Capacitive)

A minimalist monochrome PlatformIO / Arduino firmware for your Cheap Yellow Display (CYD) 2.4" Capacitive Touch board:

- 🎵 **Music Now Playing** — iOS via BLE Apple Media Service (AMS), Android via Classic BT AVRCP
- ⏰ **Clock** — NTP-synced, auto-updates
- 🖤 **Minimalist Black & White UI** — Modern card player layout inspired by clean hardware typography
- 👆 **Touch Controls** — Prev, Play/Pause (circular button), Next

---

## Hardware

| Component | Details |
|-----------|---------|
| MCU | ESP32-WROOM-32 |
| Display | ILI9341 2.4" IPS 240×320, SPI |
| Touch | CST820 capacitive multi-touch, I2C |
| Board | ESP32-2432S024C (CYD) |
| Battery Support | Onboard LiPo charger via 2-pin JST 1.25mm |

### Pin Map

| Signal | GPIO |
|--------|------|
| LCD MOSI | 13 |
| LCD MISO | 12 |
| LCD SCLK | 14 |
| LCD CS | 15 |
| LCD DC | 2 |
| LCD BL | 21 |
| Touch SDA | 33 |
| Touch SCL | 32 |
| Touch INT | 36 |
| Touch RST | 25 |

---

## First-Time Setup

### 1. Install PlatformIO

```bash
pip install platformio
# or use the VSCode PlatformIO extension
```

### 2. Configure WiFi & Timezone in `src/config.h`

```cpp
#define WIFI_SSID       "YourSSID"
#define WIFI_PASSWORD   "YourPassword"
#define NTP_UTC_OFFSET  19800        // seconds — adjust for your timezone
                                     // e.g. UTC+5:30 (IST) = 19800
                                     //      UTC+1 (BST) = 3600
                                     //      UTC-5 (EST) = -18000
```

### 3. Build & Flash

```bash
cd esp32-desk-buddy
pio run --target upload
pio device monitor   # 115200 baud
```

---

## Pairing

### iOS (Apple Music / Spotify / YouTube Music / Podcasts)
1. Open **Settings → Bluetooth** on your iPhone
2. Pair with **"DeskBuddy"**
3. Open any audio app — metadata appears automatically via Apple Media Service (AMS)

### Android
1. Open **Settings → Bluetooth** on your Android device
2. Pair with **"DeskBuddy BT"**
3. Play music — metadata arrives via AVRCP

---

## Screen Layouts

### Idle Screen (Dark / Minimalist)
- Full black background
- Large bold digital clock (NTP synced)
- Formatted date
- Subdued "WAITING FOR MUSIC" status

### Music Screen (High Contrast Black & White)
- Clean white background
- Device source pill indicator (`IOS . BT` / `ANDROID . BT`)
- Artist name in bold uppercase typography
- Track title and album in neutral tones
- Minimal progress rule
- Touch controls: `|<` (Prev), `(||) / (>)` (Circular Play/Pause), `>|` (Next)

---

## Library Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| LovyanGFX | ^1.1.16 | High-speed display driver |
| TAMC_CST820 | ^1.0.2 | Capacitive touch driver |
| NTPClient | ^3.2.1 | Time sync |
| NimBLE-Arduino | ^1.4.2 | iOS BLE / AMS |
| ESP32-A2DP | ^1.8.6 | Android A2DP sink / AVRCP |
