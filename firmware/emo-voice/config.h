#pragma once
// ============================================================
//  CẤU HÌNH — sửa các giá trị bên dưới cho khớp phần cứng/mạng
// ============================================================

// ---- Wi-Fi (BẮT BUỘC dùng băng 2.4GHz, ESP32-S3 không bắt 5GHz) ----
// BẢO MẬT: điền SSID/mật khẩu thật vào file secrets.h (đã .gitignore — KHÔNG lên GitHub).
// Mẫu secrets.h:
//   #define WIFI_SSID     "Ten_WiFi_2.4G"
//   #define WIFI_PASSWORD "matkhau"
// Không có secrets.h thì dùng giá trị mặc định bên dưới (chỉ để build thử).
#if __has_include("secrets.h")
  #include "secrets.h"
#endif
#ifndef WIFI_SSID
  #define WIFI_SSID     "TEN_WIFI_2.4GHz"
  #define WIFI_PASSWORD "MAT_KHAU_WIFI"
#endif

// ============================================================
//  ÂM THANH — board ES3C28P dùng codec ES8311 (I2C) + amp FM8002E, KHÔNG phải MAX98357A.
//  Chân cố định theo tài liệu board (lcdwiki / BSP ngttai).
// ============================================================
#define I2S_MCLK 4    // ES8311: MCLK (master clock)
#define I2S_BCLK 5    // ES8311: BCLK (bit clock)
#define I2S_DOUT 6    // ES8311: DSDIN (data ra loa)
#define I2S_LRC  7    // ES8311: LRCK (word select)
#define I2S_DIN  8    // ES8311: ASDOUT (data từ mic — chưa dùng)

#define PIN_AMP_EN   1    // Bật amp: mức THẤP = bật (active low)
#define ES8311_SDA   16   // I2C của codec (chung bus với cảm ứng)
#define ES8311_SCL   15
// Âm lượng analog codec 0..100. Để hơi thấp cho SẠCH, hết "nhựa" (méo).
// Chỉnh live bằng thanh trượt trên web robot (không cần nạp lại).
#define ES8311_VOL   55

// ---- Âm lượng số của thư viện: 0..21 (để vừa, tránh méo tín hiệu số) ----
#define TTS_VOLUME 16

// ============================================================
//  CHỌN NGUỒN TTS
//    1 = server VieNeu-TTS tự host (giọng Việt tự nhiên 48kHz, có thể clone)
//    0 = Google Translate TTS (không cần server riêng)
// ============================================================
#define USE_VIENEU 1

// ---- Server VieNeu-TTS (dùng khi USE_VIENEU = 1) ----
// IP của máy/VPS đang chạy `docker compose up` trong thư mục vieneu-tts-web.
// Robot và máy chủ phải cùng mạng LAN. (IP máy bạn trên WiFi "lht".)
#define VIENEU_HOST "192.168.43.37"
#define VIENEU_PORT 8000
// Giọng: để trống "" = giọng mặc định của server.
// Xem danh sách id giọng tại: http://VIENEU_HOST:VIENEU_PORT/api/voices
#define VIENEU_VOICE ""

// ---- Ngôn ngữ TTS (Google, dùng khi USE_VIENEU = 0): "vi" = tiếng Việt ----
#define TTS_LANG "vi"

// ---- Độ dài tối đa mỗi đoạn gửi lên Google TTS (byte, < ~200) ----
#define TTS_CHUNK_MAX 180
