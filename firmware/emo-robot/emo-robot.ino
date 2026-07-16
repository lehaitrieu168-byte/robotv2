// ============================================================
//  EMO robot — GỘP: Mắt biểu cảm + Giọng nói tiếng Việt (VieNeu-TTS)
//  Board : ESP32-S3 ES3C28P/ES3N28P (8MB PSRAM - bật OPI PSRAM trong Tools)
//  Màn   : ILI9341 2.8" (LovyanGFX, SPI2: DC=46 CS=10 SCK=12 MOSI=11 BL=45)
//  Loa   : codec ES8311 (I2C 16/15) + amp FM8002E -> I2S (MCLK=4 BCLK=5 DOUT=6 LRC=7), AMP_EN=1 (low)
//  TTS   : server VieNeu-TTS tự host (USE_VIENEU) hoặc Google TTS
//  Chia nhân: audio chạy task riêng CORE 0 (mượt), mắt+web chạy CORE 1.
// ============================================================
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <queue>
#include "Audio.h"              // ESP32-audioI2S (schreibfaul1)
#include "es8311.h"             // driver codec ES8311 (I2C)
#include "display-config.h"     // LGFX (LovyanGFX) — khai báo chân màn hình
#include "robot-eyes.h"         // engine mắt biểu cảm
#include "config.h"             // WiFi/audio/VieNeu + secrets.h
#include "web_page.h"           // trang web nhỏ để gõ chữ

LGFX      gfx;
RobotEyes eyes;
Audio     audio;
ES8311    es8311;
WebServer server(80);

// Hàng đợi đoạn cần đọc, dùng chung giữa core 1 (web) và core 0 (audio) -> cần khoá.
struct Utterance { String text; String voice; };
std::queue<Utterance> speechQueue;
SemaphoreHandle_t queueMutex;
volatile bool g_speaking   = false;   // audio task báo đang phát
volatile bool g_stopAudio  = false;   // web yêu cầu dừng

// Vòng cảm xúc demo cho mắt
const Mood cycle[] = { MOOD_NEUTRAL, MOOD_HAPPY, MOOD_SURPRISED, MOOD_ANGRY,
                       MOOD_TIRED, MOOD_LOVE, MOOD_SAD, MOOD_SLEEPY };
const int  N_MOOD  = sizeof(cycle) / sizeof(cycle[0]);
int cycleIdx = 0;
uint32_t nextMoodMs = 0;

void nextMood(uint32_t now, uint32_t holdMs) {
  cycleIdx = (cycleIdx + 1) % N_MOOD;
  eyes.setMood(cycle[cycleIdx]);
  nextMoodMs = now + holdMs;
}

// ---------- Tách text dài thành đoạn ngắn, đẩy vào hàng đợi (có khoá) ----------
void enqueueText(const String& text, const String& voice) {
  int start = 0, n = text.length();
  while (start < n) {
    int end = start + TTS_CHUNK_MAX;
    if (end < n) {
      int cut = -1;
      for (int i = end; i > start; i--) {
        char c = text[i];
        if (c==' '||c=='.'||c==','||c=='!'||c=='?'||c=='\n') { cut = i; break; }
      }
      if (cut > start) end = cut + 1;
    } else end = n;
    String chunk = text.substring(start, end); chunk.trim();
    if (chunk.length()) {
      xSemaphoreTake(queueMutex, portMAX_DELAY);
      speechQueue.push({chunk, voice});
      xSemaphoreGive(queueMutex);
    }
    start = end;
  }
}

// Percent-encode UTF-8 cho URL (KHÔNG đặt tên biến HEX — Arduino có macro HEX).
String urlEncode(const String& s) {
  static const char* HEXDIG = "0123456789ABCDEF";
  String out;
  for (size_t i = 0; i < s.length(); i++) {
    uint8_t c = (uint8_t)s[i];
    if ((c>='0'&&c<='9')||(c>='A'&&c<='Z')||(c>='a'&&c<='z')||c=='-'||c=='_'||c=='.'||c=='~')
      out += (char)c;
    else { out += '%'; out += HEXDIG[c>>4]; out += HEXDIG[c&0x0F]; }
  }
  return out;
}

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

// ---------- TASK AUDIO (CORE 0): nuôi buffer I2S + đọc hàng đợi ----------
void audioTask(void*) {
  for (;;) {
    audio.loop();
    if (g_stopAudio) { audio.stopSong(); g_stopAudio = false; }
    g_speaking = audio.isRunning();
    if (!audio.isRunning()) {
      Utterance u; bool have = false;
      xSemaphoreTake(queueMutex, portMAX_DELAY);
      if (!speechQueue.empty()) { u = speechQueue.front(); speechQueue.pop(); have = true; }
      xSemaphoreGive(queueMutex);
      if (have) speakChunk(u.text, u.voice);
    }
    vTaskDelay(1);
  }
}

// ---------- Web handlers (CORE 1) ----------
void handleRoot()   { server.send_P(200, "text/html; charset=utf-8", INDEX_HTML); }

void handleVoices() {
#if USE_VIENEU
  HTTPClient http;
  http.begin(String("http://") + VIENEU_HOST + ":" + VIENEU_PORT + "/api/voices");
  int code = http.GET();
  String body = (code == 200) ? http.getString() : String("{\"voices\":[]}");
  http.end();
  server.send(code == 200 ? 200 : 502, "application/json; charset=utf-8", body);
#else
  server.send(200, "application/json; charset=utf-8", "{\"voices\":[]}");
#endif
}

void handleSay() {
  if (!server.hasArg("text")) { server.send(400, "text/plain; charset=utf-8", "Thiếu text"); return; }
  String voice = server.hasArg("voice") ? server.arg("voice") : String("");
  enqueueText(server.arg("text"), voice);
  server.send(200, "text/plain; charset=utf-8", "OK, đang đọc");
}

void handleStop() {
  xSemaphoreTake(queueMutex, portMAX_DELAY);
  while (!speechQueue.empty()) speechQueue.pop();
  xSemaphoreGive(queueMutex);
  g_stopAudio = true;
  server.send(200, "text/plain; charset=utf-8", "Đã dừng");
}

void setup() {
  Serial.begin(115200);
  if (!psramFound())
    Serial.println("[CANH BAO] Khong thay PSRAM! Tools -> PSRAM -> OPI PSRAM.");

  // Màn hình + mắt trước (để không đen màn lúc nối WiFi)
  gfx.init(); gfx.setRotation(1); gfx.setBrightness(200);
  eyes.begin(&gfx); eyes.setMood(MOOD_NEUTRAL); eyes.update(millis());

  // WiFi
  WiFi.mode(WIFI_STA); WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Ket noi Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) { delay(400); Serial.print("."); eyes.update(millis()); }
  Serial.print("\nDa ket noi. IP robot: http://"); Serial.println(WiFi.localIP());

  // Loa: bật amp + init codec ES8311 (I2C) trước, rồi mới cấu hình I2S
  pinMode(PIN_AMP_EN, OUTPUT);
  digitalWrite(PIN_AMP_EN, LOW);                 // active low -> bật amp FM8002E
  if (es8311.begin(ES8311_SDA, ES8311_SCL, 400000)) {
    es8311.setVolume(ES8311_VOL);
    Serial.println("[ES8311] init OK");
  } else {
    Serial.println("[ES8311] init FAIL - kiem tra I2C codec");
  }
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT, I2S_MCLK);   // có MCLK cho ES8311
  audio.setVolume(TTS_VOLUME);

  // Web server
  server.on("/", handleRoot);
  server.on("/say", handleSay);
  server.on("/voices", handleVoices);
  server.on("/stop", handleStop);
  server.begin();

  // Khoá hàng đợi + task audio trên CORE 0
  queueMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(audioTask, "audio", 16384, NULL, 2, NULL, 0);

  // Lời chào để test loa ngay
  enqueueText("Xin chào, tôi là EMO. Hãy gõ chữ để tôi đọc.", "");
}

void loop() {   // CORE 1: mắt + web
  uint32_t now = millis();

  // (Board ES3C28P dùng cảm ứng điện dung I2C — chưa dùng ở bản này)

  // Đang nói -> cười; nói xong -> quay lại vòng cảm xúc
  static bool wasSpeaking = false;
  if (g_speaking && !wasSpeaking) eyes.setMood(MOOD_HAPPY);
  if (!g_speaking && wasSpeaking) nextMoodMs = now;
  wasSpeaking = g_speaking;
  if (!g_speaking && now > nextMoodMs) { nextMood(now, 3000); eyes.look(0, 0); }

  eyes.update(now);
  server.handleClient();
}

void audio_info(const char* info) { Serial.print("[audio] "); Serial.println(info); }
