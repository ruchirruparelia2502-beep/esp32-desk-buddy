#pragma once

// ============================================================
//  DisplaySetup.h — LovyanGFX custom LGFX class for
//  ILI9341 (SPI) + XPT2046 Touch on ESP32-2432S024 CYD
// ============================================================

#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
public:
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Light_PWM     _light_instance;
  lgfx::Touch_XPT2046 _touch_instance;

  LGFX() {
    // --- SPI Bus (HSPI on ESP32) ---
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host   = HSPI_HOST;  // SPI2_HOST (GPIO 14, 13, 12, 15)
      cfg.spi_mode   = 0;
      cfg.freq_write = 40000000;   // 40 MHz write
      cfg.freq_read  = 16000000;   // 16 MHz read
      cfg.spi_3wire  = false;
      cfg.use_lock   = true;
      cfg.dma_channel= SPI_DMA_CH_AUTO;
      cfg.pin_sclk   = 14;   // SCLK
      cfg.pin_mosi   = 13;   // MOSI
      cfg.pin_miso   = 12;   // MISO
      cfg.pin_dc     = 2;    // DC
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    // --- Panel ---
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs   = 15;   // CS
      cfg.pin_rst  = -1;   // RST (tied to EN / hardware reset)
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
      cfg.readable       = false;
      cfg.invert         = false;  // Standard ILI9341 orientation
      cfg.rgb_order      = false;
      cfg.dlen_16bit     = false;
      cfg.bus_shared     = true;
      _panel_instance.config(cfg);
    }

    // --- Backlight (PWM on GPIO 27) ---
    {
      auto cfg = _light_instance.config();
      cfg.pin_bl      = 27;       // CYD 2.4" backlight is GPIO 27
      cfg.invert      = false;    // active HIGH
      cfg.freq        = 44100;
      cfg.pwm_channel = 7;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    // --- Touch (XPT2046 SPI on CYD) ---
    {
      auto cfg = _touch_instance.config();
      cfg.x_min      =  300;
      cfg.x_max      = 3900;
      cfg.y_min      = 3700;
      cfg.y_max      =  200;
      cfg.pin_int    = 36;
      cfg.bus_shared = false;
      cfg.spi_host   = -1; // -1: use software SPI for XPT2046
      cfg.pin_sclk   = 25;
      cfg.pin_mosi   = 32;
      cfg.pin_miso   = 39;
      cfg.pin_cs     = 33;
      cfg.freq       = 2000000;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }
};

extern LGFX display;
