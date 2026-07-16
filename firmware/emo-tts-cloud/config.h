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

// ============================================================
//  CHỌN NGUỒN TTS
//    1 = server VieNeu-TTS tự host (giọng Việt tự nhiên 48kHz, có thể clone)
//    0 = Google Translate TTS (không cần server riêng)
// ============================================================
#define USE_VIENEU 1

// ---- Server VieNeu-TTS (dùng khi USE_VIENEU = 1) ----
// IP của máy/VPS đang chạy `docker compose up` trong thư mục vieneu-tts-web.
// Robot và máy chủ phải cùng mạng LAN.
#define VIENEU_HOST "192.168.1.10"
#define VIENEU_PORT 8000
// Giọng: để trống "" = giọng mặc định của server.
// Xem danh sách id giọng tại: http://VIENEU_HOST:VIENEU_PORT/api/voices
#define VIENEU_VOICE ""

// ---- Ngôn ngữ TTS (Google, dùng khi USE_VIENEU = 0): "vi" = tiếng Việt ----
#define TTS_LANG "vi"

// ---- Độ dài tối đa mỗi đoạn gửi lên Google TTS (byte, < ~200) ----
#define TTS_CHUNK_MAX 180
