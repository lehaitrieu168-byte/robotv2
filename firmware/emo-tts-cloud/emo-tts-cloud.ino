// ============================================================
//  EMO robot — Text-to-Speech tiếng Việt (gõ chữ tùy ý)
//  Board : ESP32-S3 N16R8 (16MB flash + 8MB PSRAM)
//  Audio : I2S -> MAX98357A -> loa
//  TTS   : USE_VIENEU=1 -> server VieNeu-TTS tự host (giọng Việt tự nhiên)
//          USE_VIENEU=0 -> Google Translate TTS
//          Cả hai đều stream qua thư viện ESP32-audioI2S
//  Nhập  : trang web nhỏ ngay trên ESP32 (mở bằng điện thoại/PC)
// ============================================================
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <queue>
#include "Audio.h"          // ESP32-audioI2S (schreibfaul1) — cài qua Library Manager
#include "config.h"
#include "web_page.h"

Audio audio;
WebServer server(80);

// Mỗi đoạn cần đọc kèm theo giọng (voice) muốn dùng. voice rỗng = giọng mặc định.
struct Utterance { String text; String voice; };
std::queue<Utterance> speechQueue;   // các đoạn ngắn, đọc lần lượt cho liền mạch

// Chia text dài thành đoạn <= TTS_CHUNK_MAX byte.
// Chỉ cắt ở khoảng trắng / dấu câu (ký tự ASCII) nên KHÔNG làm vỡ ký tự tiếng Việt UTF-8.
void enqueueText(const String& text, const String& voice) {
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
    if (chunk.length() > 0) speechQueue.push({chunk, voice});
    start = end;
  }
}

// Percent-encode chuỗi UTF-8 để nhét vào URL (?text=...). Byte tiếng Việt (>127) -> %XX.
String urlEncode(const String& s) {
  static const char* HEX = "0123456789ABCDEF";
  String out;
  for (size_t i = 0; i < s.length(); i++) {
    uint8_t c = (uint8_t)s[i];
    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z') || c == '-' || c == '_' || c == '.' || c == '~') {
      out += (char)c;
    } else {
      out += '%';
      out += HEX[c >> 4];
      out += HEX[c & 0x0F];
    }
  }
  return out;
}

// Phát một đoạn text bằng giọng cho trước: qua server VieNeu-TTS hoặc Google TTS.
void speakChunk(const String& text, const String& voice) {
#if USE_VIENEU
  String v = voice.length() ? voice : String(VIENEU_VOICE);
  String url = String("http://") + VIENEU_HOST + ":" + VIENEU_PORT +
               "/api/say?voice=" + urlEncode(v) + "&text=" + urlEncode(text);
  audio.connecttohost(url.c_str());
#else
  audio.connecttospeech(text.c_str(), TTS_LANG);
#endif
}

void handleRoot() { server.send_P(200, "text/html; charset=utf-8", INDEX_HTML); }

// Proxy danh sách giọng từ server VieNeu-TTS -> trang web robot (same-origin, khỏi CORS).
void handleVoices() {
#if USE_VIENEU
  HTTPClient http;
  String url = String("http://") + VIENEU_HOST + ":" + VIENEU_PORT + "/api/voices";
  http.begin(url);
  int code = http.GET();
  String body = (code == 200) ? http.getString() : String("{\"voices\":[]}");
  http.end();
  server.send(code == 200 ? 200 : 502, "application/json; charset=utf-8", body);
#else
  server.send(200, "application/json; charset=utf-8", "{\"voices\":[]}");
#endif
}

void handleSay() {
  if (!server.hasArg("text")) { server.send(400, "text/plain; charset=utf-8", "Thiếu tham số text"); return; }
  String text  = server.arg("text");           // đã được URL-decode về UTF-8
  String voice = server.hasArg("voice") ? server.arg("voice") : String("");
  enqueueText(text, voice);
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
#if USE_VIENEU
  Serial.printf("[TTS] Nguon: VieNeu-TTS server http://%s:%d\n", VIENEU_HOST, VIENEU_PORT);
#else
  Serial.println("[TTS] Nguon: Google Translate TTS");
#endif

  // Web server
  server.on("/", handleRoot);
  server.on("/say", handleSay);
  server.on("/voices", handleVoices);
  server.on("/stop", handleStop);
  server.begin();

  // Câu chào lúc khởi động để kiểm tra loa ngay (giọng mặc định)
  enqueueText("Xin chào, tôi là EMO. Hãy gõ chữ để tôi đọc.", "");
}

void loop() {
  audio.loop();
  server.handleClient();

  // Đọc xong đoạn hiện tại thì lấy đoạn kế trong hàng đợi
  if (!audio.isRunning() && !speechQueue.empty()) {
    Utterance u = speechQueue.front();
    speechQueue.pop();
    speakChunk(u.text, u.voice);
  }
}

// (Tùy chọn) log trạng thái ra Serial để gỡ lỗi
void audio_info(const char* info) { Serial.print("[audio] "); Serial.println(info); }
