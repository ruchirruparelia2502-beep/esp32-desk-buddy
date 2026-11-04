#pragma once

// ============================================================
//  DisplaySetup.h — LovyanGFX custom LGFX class for
//  ILI9341 (SPI) on ESP32-2432S024C
// ============================================================

#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
public:
  // Bus (SPI)
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Light_PWM     _light_instance;
  lgfx::Touch_CST816S _touch_instance; // placeholder – touch is handled separately

  LGFX() {
    // --- SPI Bus ---
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host   = VSPI_HOST;  // SPI2_HOST
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
      cfg.pin_rst  = -1;   // RST not connected
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
      cfg.readable       = true;
      cfg.invert         = true;   // IPS panel needs colour inversion
      cfg.rgb_order      = false;
      cfg.dlen_16bit     = false;
      cfg.bus_shared     = false;
      _panel_instance.config(cfg);
    }

    // --- Backlight (PWM) ---
    {
      auto cfg = _light_instance.config();
      cfg.pin_bl      = 21;
      cfg.invert      = false;    // active HIGH
      cfg.freq        = 44100;
      cfg.pwm_channel = 7;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    setPanel(&_panel_instance);
  }
};

// Global display object – defined in main.cpp
extern LGFX display;
