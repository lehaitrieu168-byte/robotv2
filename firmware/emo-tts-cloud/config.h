#pragma once
// ============================================================
//  CẤU HÌNH — sửa các giá trị bên dưới cho khớp phần cứng/mạng
// ============================================================

// ---- Wi-Fi (BẮT BUỘC dùng băng 2.4GHz, ESP32-S3 không bắt 5GHz) ----
#define WIFI_SSID     "TEN_WIFI_2.4GHz"
#define WIFI_PASSWORD "MAT_KHAU_WIFI"

// ---- Chân I2S nối tới MAX98357A ----
// Đổi cho khớp dây bạn đã cắm.
// TRÁNH các chân flash/PSRAM (GPIO26–37) trên S3 N16R8, và chân nạp (0/3/45/46).
#define I2S_BCLK 5   // MAX98357A: BCLK
#define I2S_LRC  6   // MAX98357A: LRC (WS / LRCLK)
#define I2S_DOUT 7   // MAX98357A: DIN

// ---- Âm lượng: 0..21 ----
#define TTS_VOLUME 12

// ---- Ngôn ngữ TTS (Google): "vi" = tiếng Việt ----
#define TTS_LANG "vi"

// ---- Độ dài tối đa mỗi đoạn gửi lên Google TTS (byte, < ~200) ----
#define TTS_CHUNK_MAX 180
