// ============================================================
//  EMO robot — Text-to-Speech tiếng Việt (đám mây, gõ chữ tùy ý)
//  Board : ESP32-S3 N16R8 (16MB flash + 8MB PSRAM)
//  Audio : I2S -> MAX98357A -> loa
//  TTS   : Google Translate TTS qua thư viện ESP32-audioI2S
//  Nhập  : trang web nhỏ ngay trên ESP32 (mở bằng điện thoại/PC)
// ============================================================
#include <WiFi.h>
#include <WebServer.h>
#include <queue>
#include "Audio.h"          // ESP32-audioI2S (schreibfaul1) — cài qua Library Manager
#include "config.h"
#include "web_page.h"

Audio audio;
WebServer server(80);
std::queue<String> speechQueue;   // các đoạn ngắn, đọc lần lượt cho liền mạch

// Chia text dài thành đoạn <= TTS_CHUNK_MAX byte.
// Chỉ cắt ở khoảng trắng / dấu câu (ký tự ASCII) nên KHÔNG làm vỡ ký tự tiếng Việt UTF-8.
void enqueueText(const String& text) {
  int start = 0, n = text.length();
  while (start < n) {
    int end = start + TTS_CHUNK_MAX;
    if (end >= n) {
      end = n;
    } else {
      int cut = -1;
      for (int i = end; i > start; i--) {
        char c = text[i];
        if (c == ' ' || c == '.' || c == ',' || c == '!' || c == '?' || c == '\n') { cut = i; break; }
      }
      if (cut > start) end = cut + 1;
    }
    String chunk = text.substring(start, end);
    chunk.trim();
    if (chunk.length() > 0) speechQueue.push(chunk);
    start = end;
  }
}

void handleRoot() { server.send_P(200, "text/html; charset=utf-8", INDEX_HTML); }

void handleSay() {
  if (!server.hasArg("text")) { server.send(400, "text/plain; charset=utf-8", "Thiếu tham số text"); return; }
  String text = server.arg("text");           // đã được URL-decode về UTF-8
  enqueueText(text);
  server.send(200, "text/plain; charset=utf-8", "OK, đang đọc: " + text);
}

void handleStop() {
  while (!speechQueue.empty()) speechQueue.pop();
  audio.stopSong();
  server.send(200, "text/plain; charset=utf-8", "Đã dừng");
}

void setup() {
  Serial.begin(115200);
  delay(300);
  if (!psramFound())
    Serial.println("[CANH BAO] Khong thay PSRAM! Vao Tools -> PSRAM -> 'OPI PSRAM'.");

  // Kết nối Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Ket noi Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) { delay(400); Serial.print("."); }
  Serial.println();
  Serial.print("Da ket noi. Mo trinh duyet den: http://");
  Serial.println(WiFi.localIP());

  // Khởi tạo I2S -> MAX98357A
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(TTS_VOLUME);          // 0..21

  // Web server
  server.on("/", handleRoot);
  server.on("/say", handleSay);
  server.on("/stop", handleStop);
  server.begin();

  // Câu chào lúc khởi động để kiểm tra loa ngay
  enqueueText("Xin chào, tôi là EMO. Hãy gõ chữ để tôi đọc.");
}

void loop() {
  audio.loop();
  server.handleClient();

  // Đọc xong đoạn hiện tại thì lấy đoạn kế trong hàng đợi
  if (!audio.isRunning() && !speechQueue.empty()) {
    String chunk = speechQueue.front();
    speechQueue.pop();
    audio.connecttospeech(chunk.c_str(), TTS_LANG);
  }
}

// (Tùy chọn) log trạng thái ra Serial để gỡ lỗi
void audio_info(const char* info) { Serial.print("[audio] "); Serial.println(info); }
