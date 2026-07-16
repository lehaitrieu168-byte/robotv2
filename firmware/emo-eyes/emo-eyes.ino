// ============================================================
//  EMO robot — Mắt biểu cảm trên màn 2.8" ILI9341 + cảm ứng
//  Board : ESP32-S3 N16R8 (cần bật OPI PSRAM trong Tools)
//  Thư viện: LovyanGFX (cài qua Library Manager)
//  Demo  : tự đổi cảm xúc mỗi 3s; CHẠM màn -> đổi cảm xúc + liếc theo tay
// ============================================================
#include "display-config.h"
#include "robot-eyes.h"

LGFX gfx;
RobotEyes eyes;

// Vòng cảm xúc để demo (gồm cả LOVE / SAD / SLEEPY mới)
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

void setup() {
  Serial.begin(115200);
  if (!psramFound())
    Serial.println("[CANH BAO] Khong thay PSRAM! Vao Tools -> PSRAM -> 'OPI PSRAM'.");
  gfx.init();
  gfx.setRotation(1);        // xoay ngang 320x240
  gfx.setBrightness(200);
  eyes.begin(&gfx);          // tạo sprite SAU khi đã xoay để lấy đúng 320x240
  eyes.setMood(MOOD_NEUTRAL);
}

void loop() {
  uint32_t now = millis();

  // Chạm màn -> đổi cảm xúc ngay + liếc mắt về phía điểm chạm
  int32_t tx, ty;
  if (gfx.getTouch(&tx, &ty)) {
    nextMood(now, 4000);
    eyes.look((tx - gfx.width() / 2) / 8, (ty - gfx.height() / 2) / 8);
    delay(200);              // chống 1 lần chạm bị đếm nhiều lần
  }

  // Tự đổi cảm xúc định kỳ (demo)
  if (now > nextMoodMs) { nextMood(now, 3000); eyes.look(0, 0); }

  eyes.update(now);
}
