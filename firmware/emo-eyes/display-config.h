#pragma once
// Cấu hình màn 2.8" ILI9341 + cảm ứng XPT2046 cho ESP32-S3 bằng LovyanGFX.
// Ưu điểm: khai báo chân NGAY TRONG CODE, không phải sửa file thư viện.
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// ===== ĐỔI CÁC CHÂN NÀY CHO KHỚP DÂY BẠN CẮM =====
// SPI dùng CHUNG cho cả màn hình và cảm ứng (chỉ khác chân CS).
// Tránh GPIO26–37 (flash/PSRAM của N16R8), 0/3/45/46 (chân nạp), 19/20 (USB),
// và 5/6/7 (đã dành cho audio I2S).
#define PIN_SCLK     12   // màn: SCK      | cảm ứng: T_CLK
#define PIN_MOSI     11   // màn: SDI/MOSI | cảm ứng: T_DIN
#define PIN_MISO     13   // màn: SDO/MISO | cảm ứng: T_DO
#define PIN_TFT_DC   14   // màn: DC / RS
#define PIN_TFT_CS   10   // màn: CS
#define PIN_TFT_RST  21   // màn: RESET
#define PIN_TFT_BL   47   // màn: LED (đèn nền) — hoặc nối thẳng 3V3
#define PIN_TOUCH_CS 9    // cảm ứng: T_CS

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341 _panel;
  lgfx::Bus_SPI       _bus;
  lgfx::Light_PWM     _light;
  lgfx::Touch_XPT2046 _touch;
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
    { auto c = _touch.config();
      c.x_min = 300; c.x_max = 3900; c.y_min = 200; c.y_max = 3900;
      c.pin_int = -1; c.bus_shared = true; c.offset_rotation = 0;
      c.spi_host = SPI2_HOST; c.freq = 1000000;
      c.pin_sclk = PIN_SCLK; c.pin_mosi = PIN_MOSI; c.pin_miso = PIN_MISO; c.pin_cs = PIN_TOUCH_CS;
      _touch.config(c); _panel.setTouch(&_touch); }
    setPanel(&_panel);
  }
};
