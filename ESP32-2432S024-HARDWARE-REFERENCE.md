# ESP32-2432S024 (CYD 2.4") Complete Hardware Reference & Pinout Guide

This document is the definitive hardware, pinout, and configuration manual for the **ESP32-2432S024** (the 2.4-inch "Cheap Yellow Display" / CYD board). Keep this file handy for all future firmware projects on this exact hardware.

---

## 📌 1. Hardware Overview

| Specification | Details |
|---|---|
| **MCU** | Espressif ESP32-D0WD-V3 (Dual-core Tensilica Xtensa 32-bit LX6 @ 240 MHz) |
| **Wireless** | Wi-Fi 802.11 b/g/n (2.4 GHz) + Bluetooth v4.2 BR/EDR & BLE |
| **Flash / RAM** | 4MB SPI Flash, 520KB SRAM |
| **Display** | 2.4-inch TFT LCD, 240 × 320 resolution, ILI9341 controller (HSPI) |
| **Touchscreen** | 4-wire Resistive touch layer driven by an onboard **XPT2046** TSSOP controller |
| **Backlight** | White LED backlight with PWM dimming via **GPIO 27** |
| **Audio** | FM8002A / SC8002B mono audio amplifier (GPIO 26) with 2-pin JST-1.25mm connector |
| **Expansion / Storage** | MicroSD (TF) card socket on shared SPI bus |
| **Onboard Sensors** | LDR photoresistor (ambient light sensor on **GPIO 34**) |
| **Status LED** | Onboard 3-color discrete RGB LED (Active-LOW: Red=4, Green=16, Blue=17) |
| **Power & Programming** | USB Type-C port via CH340 serial bridge; 2-pin battery JST connector |

---

## 🔌 2. Complete GPIO Pinout Mapping

### A. TFT Display (ILI9341 on Hardware HSPI)
> [!IMPORTANT]
> The display uses the hardware **HSPI** bus (`SPI2_HOST`).

| Signal | ESP32 GPIO | Description |
|---|---|---|
| **MOSI (SDI)** | **GPIO 13** | Hardware SPI Data Out from ESP32 |
| **MISO (SDO)** | **GPIO 12** | Hardware SPI Data In to ESP32 |
| **SCLK (SCK)** | **GPIO 14** | Hardware SPI Clock |
| **CS (TFT_CS)** | **GPIO 15** | Display Chip Select (Active LOW) |
| **DC (TFT_DC / RS)** | **GPIO 2** | Data / Command select line |
| **RST (TFT_RST)** | **-1** | Tied internally to EN / hardware reset line |
| **BL (TFT_BL)** | **GPIO 27** | Backlight PWM control (**Active HIGH**, ~44.1 kHz PWM) |

---

### B. Touch Controller (XPT2046 Resistive Touch)
> [!WARNING]
> **CRITICAL DIFFERENCE FROM 2.8" CYD:**
> On the classic 2.8" CYD (`ESP32-2432S028R`), touch is routed to separate GPIOs (25, 32, 39).
> **On this 2.4" board (`ESP32-2432S024`), the XPT2046 shares the display's hardware HSPI bus (GPIO 14, 13, 12)!**

| Signal | ESP32 GPIO | Shared / Dedicated | Notes |
|---|---|---|---|
| **T_CLK (DCLK)** | **GPIO 14** | **Shared with Display** | HSPI Clock |
| **T_DIN (MOSI)** | **GPIO 13** | **Shared with Display** | HSPI Master Out / Slave In |
| **T_DO (MISO)** | **GPIO 12** | **Shared with Display** | HSPI Master In / Slave Out |
| **T_CS** | **GPIO 33** | **Dedicated** | Touch Chip Select (Active LOW) |
| **T_IRQ (\*PENIRQ)**| **GPIO 36** | **Dedicated** | Pen Interrupt. Input-only pad (no internal PU). In libraries, set `pin_int = -1` or use SPI polling to prevent blocking on floating states. |

*Recommended Touch SPI Clock:* **`2.5 MHz`** (`2500000 Hz`).

---

### C. Onboard Peripherals & Sensors

| Peripheral | ESP32 GPIO | Notes |
|---|---|---|
| **Audio Amp (FM8002A)** | **GPIO 26** | Connected to amplifier + speaker header (DAC1 output). |
| **LDR Photoresistor** | **GPIO 34** | Analog input (`ADC1_CH6`). Input-only pin. |
| **RGB LED - Red** | **GPIO 4** | Active LOW (`0` = ON, `1` = OFF). |
| **RGB LED - Green**| **GPIO 16**| Active LOW (`0` = ON, `1` = OFF). |
| **RGB LED - Blue** | **GPIO 17**| Active LOW (`0` = ON, `1` = OFF). |
| **MicroSD Slot CS** | **GPIO 25** (or 5)| Chip Select for SD card on shared HSPI bus. |
| **Boot Button** | **GPIO 0** | Pulled high, pulls LOW when pressed. |

---

## 🛠️ 3. Arduino IDE & PlatformIO Configuration

### Arduino IDE 2.x Settings:
* **Board:** `ESP32 Dev Module` (or `jczn_2432s028r` from ESP32 board package)
* **CPU Frequency:** `240 MHz` (WiFi/BT)
* **Flash Frequency:** `80 MHz`
* **Flash Mode:** `QIO` or `DIO`
* **Flash Size:** `4MB (32Mb)`
* **Partition Scheme:** `Huge APP (3MB No OTA/1MB SPIFFS)` *(Essential when compiling Wi-Fi + Bluetooth together!)*
* **Upload Speed:** `460800` *(Stable and fast; 921600 can occasionally timeout)*
* **Port:** `/dev/cu.usbserial-110` (on macOS)

---

## 💻 4. Tested & Verified LovyanGFX Setup (`DisplaySetup.h`)

Copy-paste this directly into any future project using LovyanGFX for zero-setup display and touch:

```cpp
#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
public:
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Light_PWM     _light_instance;
  lgfx::Touch_XPT2046 _touch_instance;

  LGFX() {
    // 1. SPI Bus (Hardware HSPI on ESP32)
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host   = HSPI_HOST;   // SPI2_HOST
      cfg.spi_mode   = 0;
      cfg.freq_write = 40000000;    // 40 MHz write
      cfg.freq_read  = 16000000;    // 16 MHz read
      cfg.spi_3wire  = false;
      cfg.use_lock   = true;
      cfg.dma_channel= SPI_DMA_CH_AUTO;
      cfg.pin_sclk   = 14;          // SCLK
      cfg.pin_mosi   = 13;          // MOSI
      cfg.pin_miso   = 12;          // MISO
      cfg.pin_dc     = 2;           // DC / RS
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    // 2. Panel Config (ILI9341)
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs   = 15;            // TFT CS
      cfg.pin_rst  = -1;            // Tied to EN
      cfg.pin_busy = -1;
      cfg.memory_width  = 240;
      cfg.memory_height = 320;
      cfg.panel_width   = 240;
      cfg.panel_height  = 320;
      cfg.offset_x      = 0;
      cfg.offset_y      = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits  = 1;
      cfg.readable       = false;   // Prevent MISO contention
      cfg.invert         = false;
      cfg.rgb_order      = false;
      cfg.dlen_16bit     = false;
      cfg.bus_shared     = true;    // Shared HSPI bus with touch
      _panel_instance.config(cfg);
    }

    // 3. Backlight PWM (GPIO 27)
    {
      auto cfg = _light_instance.config();
      cfg.pin_bl      = 27;         // Backlight GPIO
      cfg.invert      = false;      // Active HIGH
      cfg.freq        = 44100;      // 44.1 kHz PWM
      cfg.pwm_channel = 7;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    // 4. XPT2046 Resistive Touch (Shared HSPI Bus)
    {
      auto cfg = _touch_instance.config();
      cfg.x_min      = 300;
      cfg.x_max      = 3900;
      cfg.y_min      = 300;
      cfg.y_max      = 3900;
      cfg.pin_int    = 36;          // PENIRQ
      cfg.bus_shared = true;        // MUST be true for shared bus
      cfg.spi_host   = HSPI_HOST;   // Same HSPI bus
      cfg.freq       = 2500000;     // 2.5 MHz for stable touch reads
      cfg.pin_cs     = 33;          // Touch CS
      cfg.pin_sclk   = 14;          // Shared SCLK
      cfg.pin_mosi   = 13;          // Shared MOSI
      cfg.pin_miso   = 12;          // Shared MISO
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }
};

extern LGFX display;
```

---

## ⚡ 5. Critical Gotchas & Best Practices

1. **Bluetooth Audio & I2S DMA Crash Prevention:**
   If using `ESP32-A2DP` as a metadata receiver without speaker output, you **MUST** disable I2S hardware initialization:
   ```cpp
   static void _dummyAudioStream(const uint8_t *data, uint32_t length) {}
   // pass false to suppress I2S hardware initialization:
   a2dp_sink.set_stream_reader(_dummyAudioStream, false);
   ```
   Without this, the ESP32 tries to claim I2S pins (GPIO 25/26/22) for DMA, resulting in reboot loops upon phone connection.

2. **Dual Bluetooth Stacks (BLE + Classic BT):**
   Do **not** run NimBLE and Bluedroid (Classic BT) concurrently on an ESP32 with 520KB SRAM unless you carefully tune the controller buffers. Use a compile-time switch (`TARGET_PHONE_MODE`) to select either iOS BLE (AMS) or Universal Classic BT (A2DP/AVRCP).

3. **Live Track Position via AVRCP:**
   To receive live progress updates from iOS or Android over Bluetooth AVRCP:
   - Request `ESP_AVRC_MD_ATTR_PLAYING_TIME` in the metadata attribute mask for total track duration.
   - Register `a2dp_sink.set_avrc_rn_play_pos_callback(callback, 1);` to stream position updates every 1 second.
   - Interpolate milliseconds locally between 1-second ticks while `isPlaying` is true for 60fps smooth progress animation.
