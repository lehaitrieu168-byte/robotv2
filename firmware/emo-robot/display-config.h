#pragma once
// Cấu hình màn 2.8" ILI9341 cho board ESP32-S3 ES3C28P/ES3N28P bằng LovyanGFX.
// Chân theo tài liệu board (lcdwiki / BSP ngttai). SPI2, không có touch SPI
// (board dùng cảm ứng điện dung FT6336 qua I2C — không cần cho phần mắt).
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// ===== Chân màn hình của board ES3C28P (CỐ ĐỊNH theo phần cứng) =====
#define PIN_SCLK    12   // LCD SCK
#define PIN_MOSI    11   // LCD MOSI
#define PIN_MISO    13   // LCD MISO
#define PIN_TFT_DC  46   // LCD DC/RS
#define PIN_TFT_CS  10   // LCD CS
#define PIN_TFT_RST -1   // RST dùng chung reset của ESP32-S3 (không phải GPIO riêng)
#define PIN_TFT_BL  45   // LCD đèn nền

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341 _panel;
  lgfx::Bus_SPI       _bus;
  lgfx::Light_PWM     _light;
public:
  LGFX() {
    { auto c = _bus.config();
      c.spi_host = SPI2_HOST; c.spi_mode = 0;
      c.freq_write = 40000000; c.freq_read = 16000000;
      c.spi_3wire = false; c.use_lock = true; c.dma_channel = SPI_DMA_CH_AUTO;
      c.pin_sclk = PIN_SCLK; c.pin_mosi = PIN_MOSI; c.pin_miso = PIN_MISO; c.pin_dc = PIN_TFT_DC;
      _bus.config(c); _panel.setBus(&_bus); }
    { auto c = _panel.config();
      c.pin_cs = PIN_TFT_CS; c.pin_rst = PIN_TFT_RST; c.pin_busy = -1;
      c.panel_width = 240; c.panel_height = 320;
      c.offset_rotation = 0; c.readable = true; c.invert = false;
      c.rgb_order = false; c.dlen_16bit = false; c.bus_shared = true;
      _panel.config(c); }
    { auto c = _light.config();
      c.pin_bl = PIN_TFT_BL; c.invert = false; c.freq = 44100; c.pwm_channel = 7;
      _light.config(c); _panel.setLight(&_light); }
    setPanel(&_panel);
  }
};
