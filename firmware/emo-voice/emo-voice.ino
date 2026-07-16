// ============================================================
//  EMO voice — TRÒ CHUYỆN GIỌNG NÓI (tự host, không XiaoZhi)
//  Board : ESP32-S3 ES3C28P (codec ES8311, mic MEMS + loa)
//  Vòng  : tự thu mic -> /api/chat-audio (Whisper nghe + Groq nghĩ + VieNeu nói)
//          -> phát câu trả lời ra loa. Đèn RGB báo trạng thái.
//  Mấu chốt: I2S FULL-DUPLEX + cấu hình ADC ES8311 SAU khi clock chạy.
// ============================================================
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <driver/i2s_std.h>
#include "es8311.h"
#include "config.h"

#define REC_SECONDS  4
#define REC_RATE     16000                 // 16kHz: Whisper thích, và dùng luôn cho phát
#define REC_SAMPLES  (REC_RATE * REC_SECONDS)
#define REC_BYTES    (REC_SAMPLES * 2)      // 16-bit mono
#define VAD_RMS      120                    // ngưỡng "có tiếng nói"

ES8311 es8311;
i2s_chan_handle_t tx_handle = NULL, rx_handle = NULL;
int16_t* recBuf = NULL;

void es8311Write(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(0x18); Wire.write(reg); Wire.write(val); Wire.endTransmission();
}
void ledShow(uint8_t r, uint8_t g, uint8_t b) { rgbLedWrite(42, r, g, b); rgbLedWrite(48, r, g, b); }

// I2S FULL-DUPLEX master: TX ra loa (DOUT), RX từ mic (DIN), MCLK giữ codec chạy.
void i2sInit(uint32_t rate) {
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle);
  i2s_std_config_t std_cfg = {};
  std_cfg.clk_cfg  = (i2s_std_clk_config_t)I2S_STD_CLK_DEFAULT_CONFIG(rate);
  std_cfg.slot_cfg = (i2s_std_slot_config_t)
      I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
  std_cfg.gpio_cfg.mclk = (gpio_num_t)I2S_MCLK;
  std_cfg.gpio_cfg.bclk = (gpio_num_t)I2S_BCLK;
  std_cfg.gpio_cfg.ws   = (gpio_num_t)I2S_LRC;
  std_cfg.gpio_cfg.dout = (gpio_num_t)I2S_DOUT;
  std_cfg.gpio_cfg.din  = (gpio_num_t)I2S_DIN;
  i2s_channel_init_std_mode(tx_handle, &std_cfg);
  i2s_channel_init_std_mode(rx_handle, &std_cfg);
  i2s_channel_enable(tx_handle);
  i2s_channel_enable(rx_handle);
}

void writeWavHeader(uint8_t* h, uint32_t dataBytes) {
  uint32_t rate = REC_RATE, byteRate = rate * 2; uint16_t block = 2, bits = 16, ch = 1, fmt = 1;
  uint32_t riff = 36 + dataBytes; uint32_t f16 = 16;
  memcpy(h, "RIFF", 4); memcpy(h + 4, &riff, 4); memcpy(h + 8, "WAVE", 4);
  memcpy(h + 12, "fmt ", 4); memcpy(h + 16, &f16, 4);
  memcpy(h + 20, &fmt, 2); memcpy(h + 22, &ch, 2); memcpy(h + 24, &rate, 4);
  memcpy(h + 28, &byteRate, 4); memcpy(h + 32, &block, 2); memcpy(h + 34, &bits, 2);
  memcpy(h + 36, "data", 4); memcpy(h + 40, &dataBytes, 4);
}

// Phát PCM 16-bit 16kHz ra loa qua I2S TX.
void playPcm(const uint8_t* pcm, size_t len) {
  ledShow(0, 0, 90);                 // xanh dương = đang nói
  digitalWrite(PIN_AMP_EN, LOW);     // bật amp (active low)
  size_t w = 0;
  while (w < len) { size_t bw = 0; i2s_channel_write(tx_handle, pcm + w, len - w, &bw, 1000); w += bw; }
  delay(60);
  digitalWrite(PIN_AMP_EN, HIGH);    // tắt amp cho đỡ nhiễu khi im
}

void chatTurn() {
  // 1) Thu 4 giây từ mic
  size_t got = 0;
  while (got < REC_BYTES) {
    size_t r = 0;
    i2s_channel_read(rx_handle, (uint8_t*)recBuf + got, REC_BYTES - got, &r, 1000);
    got += r;
  }
  // 2) Có tiếng nói không? (VAD đơn giản theo RMS)
  double sumsq = 0;
  for (size_t i = 0; i < REC_SAMPLES; i++) { int16_t v = recBuf[i]; sumsq += (double)v * v; }
  double rms = sqrt(sumsq / REC_SAMPLES);
  if (rms < VAD_RMS) { ledShow(8, 0, 0); return; }   // im lặng -> chờ
  Serial.printf("[NGHE] RMS=%.0f -> gui server\n", rms);
  ledShow(70, 35, 0);                                 // vàng = đang nghĩ

  // 3) Gói multipart WAV + POST /api/chat-audio
  String boundary = "----emovoice";
  String head = "--" + boundary + "\r\nContent-Disposition: form-data; name=\"audio\"; "
                "filename=\"mic.wav\"\r\nContent-Type: audio/wav\r\n\r\n";
  String tail = "\r\n--" + boundary + "--\r\n";
  size_t bodyLen = head.length() + 44 + REC_BYTES + tail.length();
  uint8_t* body = (uint8_t*)ps_malloc(bodyLen);
  if (!body) { Serial.println("[ERR] het RAM"); return; }
  size_t p = 0;
  memcpy(body + p, head.c_str(), head.length()); p += head.length();
  writeWavHeader(body + p, REC_BYTES); p += 44;
  memcpy(body + p, recBuf, REC_BYTES); p += REC_BYTES;
  memcpy(body + p, tail.c_str(), tail.length());

  HTTPClient http;
  http.begin(String("http://") + VIENEU_HOST + ":" + VIENEU_PORT + "/api/chat-audio");
  http.setTimeout(30000);
  http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
  int code = http.POST(body, bodyLen);
  free(body);
  Serial.printf("[HTTP] %d\n", code);

  // 4) Nhận WAV trả lời (16kHz) -> phát ra loa
  if (code == 200) {
    int len = http.getSize();
    WiFiClient* st = http.getStreamPtr();
    size_t cap = (len > 0) ? (size_t)len : 500000;
    uint8_t* reply = (uint8_t*)ps_malloc(cap);
    size_t rgot = 0; uint32_t t0 = millis();
    while ((len < 0 || rgot < (size_t)len) && rgot < cap && millis() - t0 < 25000) {
      size_t av = st->available();
      if (av) { rgot += st->readBytes(reply + rgot, min(av, cap - rgot)); t0 = millis(); }
      else { if (!http.connected() && !st->available()) break; delay(2); }
    }
    Serial.printf("[NOI] %u bytes\n", (unsigned)rgot);
    if (reply && rgot > 44) playPcm(reply + 44, rgot - 44);   // bỏ 44 byte header WAV
    if (reply) free(reply);
  }
  http.end();
  ledShow(8, 0, 0);   // về trạng thái chờ (đỏ mờ)
}

void setup() {
  Serial.begin(115200); delay(300);
  Serial.println("\n=== EMO voice - tro chuyen ===");
  if (!psramFound()) Serial.println("[CANH BAO] Khong thay PSRAM!");

  pinMode(PIN_AMP_EN, OUTPUT); digitalWrite(PIN_AMP_EN, HIGH);   // amp tắt khi chờ/thu
  recBuf = (int16_t*)ps_malloc(REC_BYTES);

  WiFi.mode(WIFI_STA); WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Ket noi Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) { delay(400); Serial.print("."); }
  Serial.printf("\nOK. IP robot: %s\n", WiFi.localIP().toString().c_str());

  // Codec + I2S: bật clock TRƯỚC rồi mới cấu hình ADC (ADC cần clock để lock)
  bool ok = es8311.begin(ES8311_SDA, ES8311_SCL, 400000);
  es8311.setSampleRate(REC_RATE);
  i2sInit(REC_RATE);
  delay(80);
  es8311Write(0x0D, 0x01); es8311Write(0x0E, 0x02);
  es8311Write(0x15, 0x40); es8311Write(0x1B, 0x0A); es8311Write(0x1C, 0x6A);
  es8311Write(0x16, 0x07); es8311Write(0x17, 0xC8); es8311Write(0x14, 0x1A);
  Serial.println(ok ? "[ES8311] OK (ADC sau clock)" : "[ES8311] FAIL");

  ledShow(8, 0, 0);
  Serial.println(">>> Cu noi vao mic, robot se tra loi <<<");
}

void loop() {
  chatTurn();
  delay(250);
}
