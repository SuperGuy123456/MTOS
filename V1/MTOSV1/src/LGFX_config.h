#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <Wire.h>
#include "TCA9554.h"

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7796  _panel;
  lgfx::Bus_SPI       _bus;
  lgfx::Light_PWM     _light;
  lgfx::Touch_FT5x06  _touch;

public:
  LGFX(void) {

    // ---- SPI BUS (LCD ST7796) ----
    {
      auto cfg = _bus.config();

      cfg.spi_host   = SPI2_HOST;
      cfg.spi_mode   = 0;
      cfg.freq_write = 40000000;

      cfg.pin_sclk = 5;   // FACTORY: SPI_SCLK
      cfg.pin_mosi = 1;   // FACTORY: SPI_MOSI
      cfg.pin_miso = 2;   // FACTORY: SPI_MISO

      cfg.pin_dc   = 3;   // FACTORY: LCD_DC

      _bus.config(cfg);
      _panel.setBus(&_bus);
    }

    // ---- LCD PANEL (ST7796) ----
    {
      auto cfg = _panel.config();

      cfg.pin_cs   = -1;  // FACTORY: tied low
      cfg.pin_rst  = -1;  // FACTORY: reset via TCA9554
      cfg.pin_busy = -1;

      cfg.memory_width  = 320;
      cfg.memory_height = 480;
      cfg.panel_width   = 320;
      cfg.panel_height  = 480;

      cfg.offset_x = 0;
      cfg.offset_y = 0;

      cfg.readable  = false;
      cfg.invert    = true;
      cfg.rgb_order = false;

      cfg.dlen_16bit = false;
      cfg.bus_shared = false;

      _panel.config(cfg);
    }

    // ---- BACKLIGHT ----
    {
      auto cfg = _light.config();

      cfg.pin_bl      = 6;     // FACTORY: GFX_BL
      cfg.invert      = false;
      cfg.freq        = 5000;
      cfg.pwm_channel = 7;

      _light.config(cfg);
      _panel.setLight(&_light);
    }

    // ---- TOUCH (FT6336 / FT6X36) ----
    {
      auto cfg = _touch.config();

      cfg.i2c_port = 0;
      cfg.pin_sda  = 8;   // FACTORY: I2C_SDA
      cfg.pin_scl  = 7;   // FACTORY: I2C_SCL

      cfg.pin_int  = -1;  // FACTORY: not used
      cfg.pin_rst  = -1;  // FACTORY: not used

      cfg.x_min = 0;
      cfg.x_max = 319;
      cfg.y_min = 0;
      cfg.y_max = 479;

      cfg.bus_shared = false;

      _touch.config(cfg);
      _panel.setTouch(&_touch);
    }

    setPanel(&_panel);
  }
};

extern LGFX lcd;
