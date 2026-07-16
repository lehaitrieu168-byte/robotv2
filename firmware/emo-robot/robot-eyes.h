#pragma once
// Engine mặt biểu cảm: vẽ mắt + miệng lên 1 sprite, đổi hình theo cảm xúc.
// Tự chớp, liếc, tween mượt; đổi cảm xúc có hiệu ứng chuyển cảnh (nảy/rung).
// Vẽ hình = face-shapes.h | Thông số từng cảm xúc = mood-params.h
#include <LovyanGFX.hpp>
#include <math.h>
#include "mood-params.h"
#include "face-shapes.h"

class RobotEyes {
public:
  void begin(lgfx::LGFX_Device* g) {
    gfx = g;
    W = gfx->width(); H = gfx->height();
    canvas.setColorDepth(16);
    canvas.setPsram(true);                 // bộ đệm nằm trong PSRAM (S3 N16R8 dư sức)
    canvas.createSprite(W, H);
    eyeColor  = canvas.color565(60, 216, 255);   // xanh cyan kiểu EMO
    heartColor= canvas.color565(255, 90, 140);    // hồng cho mắt LOVE
    tearColor = canvas.color565(120, 200, 255);    // xanh nhạt cho nước mắt
    bgColor   = 0x0000;
    baseW = W * 0.20f; baseH = H * 0.36f;   // mắt nhỏ lại chút để chừa chỗ cho miệng
    curW = tgtW = baseW; curH = tgtH = baseH;
    gap = W * 0.12f;
    eyeCenterY = (int)(H * 0.42f);           // đẩy mắt lên, miệng nằm dưới
    mood = MOOD_NEUTRAL;
    blink = 1.0f; blinkDir = 0;
    lookX = lookY = 0;
    lastMs = 0; nextBlinkMs = 1500; moodChangeMs = 0;
  }

  void setMood(Mood m) {
    mood = m;
    MoodStyle s = moodStyle(m);
    tgtW = baseW * s.wScale;
    tgtH = baseH * s.hScale;
    moodChangeMs = lastMs;                  // mốc bắt đầu hiệu ứng chuyển cảnh
  }

  void look(int dx, int dy) { lookX = dx; lookY = dy; }

  // Gọi mỗi khung hình
  void update(uint32_t now) {
    float dt = lastMs ? (now - lastMs) / 1000.0f : 0.016f;
    if (dt > 0.05f) dt = 0.05f;
    lastMs = nowMs = now;

    // Chớp mắt: khép nhanh rồi mở lại, hẹn lần chớp kế
    if (blinkDir == 0 && now > nextBlinkMs) blinkDir = -1;
    blink += blinkDir * dt * 9.0f;
    if (blink <= 0.05f) { blink = 0.05f; blinkDir = 1; }
    if (blink >= 1.0f)  { blink = 1.0f;  blinkDir = 0; nextBlinkMs = now + 1800 + (now % 2500); }

    // Tween kích thước mắt về mục tiêu của cảm xúc hiện tại
    float k = min(1.0f, dt * 10.0f);
    curW += (tgtW - curW) * k;
    curH += (tgtH - curH) * k;

    draw();
  }

private:
  void draw() {
    canvas.fillScreen(bgColor);
    MoodStyle s = moodStyle(mood);

    // --- Hiệu ứng chuyển cảnh: tr chạy 0->1 sau khi đổi cảm xúc ---
    float tr = 1.0f;
    if (moodChangeMs) {
      float e = (float)(nowMs - moodChangeMs) / TRANS_MS;
      tr = e < 0 ? 0 : (e > 1 ? 1 : e);
    }
    float wave   = sinf(tr * PI);                       // 0 -> 1 -> 0: phồng rồi xẹp
    float ew     = curW * (1.0f + wave * s.pop * 0.6f); // ngang nảy ít
    float eh     = curH * (1.0f + wave * s.pop);        // dọc nảy nhiều -> squash&stretch
    int   shakeX = (int)(s.shake * sinf(tr * PI * 8.0f) * (1.0f - tr));  // rung tắt dần

    int cy  = eyeCenterY + lookY;
    int cxL = W / 2 - (int)(gap / 2) - (int)(ew / 2) + lookX + shakeX;
    int cxR = W / 2 + (int)(gap / 2) + (int)(ew / 2) + lookX + shakeX;

    drawEyes(s, cxL, cxR, cy, ew, eh);

    // --- Miệng nằm dưới, bám theo kích thước mắt ---
    int mouthY = cy + (int)(eh * 0.5f) + (int)(H * 0.12f);
    if (mouthY > H - 26) mouthY = H - 26;
    drawMouth(canvas, W / 2 + lookX + shakeX, mouthY, (int)(baseW * 1.15f), s.mouth, eyeColor);

    // --- Phụ kiện cảm xúc ---
    if (s.tear) drawFallingTear(cxL, cy, ew, eh);
    if (s.zzz)  drawZzz(canvas, cxR + (int)(ew * 0.55f), cy - (int)(eh * 0.7f), eyeColor);

    canvas.pushSprite(gfx, 0, 0);
  }

  void drawEyes(const MoodStyle& s, int cxL, int cxR, int cy, float ew, float eh) {
    if (s.heart) {                                   // LOVE: 2 trái tim đập nhẹ
      float beat = 1.0f + 0.10f * sinf(nowMs / 140.0f);
      drawHeart(canvas, cxL, cy, ew * beat, heartColor);
      drawHeart(canvas, cxR, cy, ew * beat, heartColor);
    } else if (s.closed) {                           // SLEEPY: 2 mắt nhắm
      drawClosedEye(canvas, cxL, cy, ew, eyeColor);
      drawClosedEye(canvas, cxR, cy, ew, eyeColor);
    } else {                                         // còn lại: mắt bo tròn + chớp
      float ehBlink = eh * blink;
      drawEyeShape(canvas, cxL, cy, ew, ehBlink, true,  s.lid, eyeColor, bgColor);
      drawEyeShape(canvas, cxR, cy, ew, ehBlink, false, s.lid, eyeColor, bgColor);
    }
  }

  // Nước mắt rơi theo chu kỳ dưới mắt trái
  void drawFallingTear(int cxL, int cy, float ew, float eh) {
    float tp = (nowMs % 1600) / 1600.0f;             // 0..1 mỗi 1.6s
    if (tp > 0.92f) return;
    int tx = cxL - (int)(ew * 0.12f);
    int ty = cy + (int)(eh * 0.35f) + (int)(tp * H * 0.22f);
    drawTear(canvas, tx, ty, 5, tearColor);
  }

  static constexpr float TRANS_MS = 450.0f;          // thời lượng hiệu ứng chuyển cảnh

  lgfx::LGFX_Device* gfx = nullptr;
  lgfx::LGFX_Sprite  canvas;
  int W = 0, H = 0, gap = 0, eyeCenterY = 0;
  float baseW = 0, baseH = 0, curW = 0, curH = 0, tgtW = 0, tgtH = 0;
  float blink = 1.0f; int blinkDir = 0;
  int lookX = 0, lookY = 0;
  Mood mood = MOOD_NEUTRAL;
  uint32_t lastMs = 0, nowMs = 0, nextBlinkMs = 0, moodChangeMs = 0;
  uint16_t eyeColor = 0xFFFF, heartColor = 0xF80F, tearColor = 0x07FF, bgColor = 0x0000;
};
