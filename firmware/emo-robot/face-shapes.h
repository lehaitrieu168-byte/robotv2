#pragma once
// Các hàm vẽ hình MẶT bằng lệnh vector (không dùng ảnh bitmap).
// Tách riêng để robot-eyes.h gọn: file này chỉ "vẽ hình", không giữ trạng thái.
#include <LovyanGFX.hpp>
#include <math.h>
#include "mood-params.h"

// --- 1 con mắt: khối bo tròn + mí khoét theo cảm xúc ---
inline void drawEyeShape(lgfx::LGFX_Sprite& c, int cx, int cy, float w, float h,
                         bool leftEye, Lid lid, uint16_t eye, uint16_t bg) {
  int hw = (int)(w / 2), hh = (int)(h / 2);
  int r  = (int)(fminf(w, h) * 0.35f);
  c.fillSmoothRoundRect(cx - hw, cy - hh, (int)w, (int)h, r, eye);

  int slant  = (int)(h * 0.6f);
  int innerX = leftEye ? (cx + hw) : (cx - hw);   // phía trong (hướng tâm mặt)
  int outerX = leftEye ? (cx - hw) : (cx + hw);   // phía ngoài
  switch (lid) {
    case LID_ANGRY:   // mí chếch xuống phía TRONG -> cau có
      c.fillTriangle(outerX, cy - hh, innerX, cy - hh, innerX, cy - hh + slant, bg); break;
    case LID_TIRED:   // mí chếch xuống phía NGOÀI -> buồn ngủ
      c.fillTriangle(outerX, cy - hh, innerX, cy - hh, outerX, cy - hh + slant, bg); break;
    case LID_SAD:     // mí chếch xuống phía NGOÀI, nhẹ hơn -> buồn
      c.fillTriangle(outerX, cy - hh, innerX, cy - hh, outerX, cy - hh + slant / 2, bg); break;
    case LID_HAPPY:   // khoét đáy cong lên -> mắt cười ^_^
      c.fillCircle(cx, cy + (int)(h * 0.55f), (int)(w * 0.62f), bg); break;
    default: break;
  }
}

// --- Mắt nhắm (SLEEPY): 1 đường bo tròn nằm ngang ---
inline void drawClosedEye(lgfx::LGFX_Sprite& c, int cx, int cy, float w, uint16_t eye) {
  int hw = (int)(w / 2), th = 5;
  c.fillSmoothRoundRect(cx - hw, cy - th / 2, (int)w, th, th / 2, eye);
}

// --- Trái tim (mắt LOVE): 2 thùy tròn phía trên + tam giác nhọn dưới ---
inline void drawHeart(lgfx::LGFX_Sprite& c, int cx, int cy, float w, uint16_t col) {
  int rr   = (int)(w * 0.26f);
  int topY = cy - (int)(w * 0.08f);
  int botY = cy + (int)(w * 0.46f);
  c.fillCircle(cx - rr, topY, rr, col);
  c.fillCircle(cx + rr, topY, rr, col);
  c.fillTriangle(cx - 2 * rr, topY, cx + 2 * rr, topY, cx, botY, col);
}

// --- Miệng: đổi hình theo cảm xúc (cung tròn / ellipse) ---
inline void drawMouth(lgfx::LGFX_Sprite& c, int mx, int my, int mw, Mouth type, uint16_t col) {
  int r = mw / 2;
  int t = r * 0.28f < 3 ? 3 : (int)(r * 0.28f);   // độ dày nét
  switch (type) {
    case MOUTH_SMILE:   // cung cong xuống dưới -> cười (U)
      c.fillArc(mx, my - r / 2, r - t, r, 25.f, 155.f, col); break;
    case MOUTH_FROWN:   // cung cong lên trên -> mếu (∩)
      c.fillArc(mx, my + r / 2, r - t, r, 205.f, 335.f, col); break;
    case MOUTH_OPEN:    // ellipse đặc -> há hốc (ngạc nhiên)
      c.fillEllipse(mx, my, (int)(mw * 0.26f), (int)(mw * 0.34f), col); break;
    case MOUTH_FLAT:    // vạch ngắn -> phẳng lì (mệt/ngủ)
      c.fillSmoothRoundRect(mx - r / 2, my - 2, r, 4, 2, col); break;
    default:            // MOUTH_LINE: vạch ngang bình thường
      c.fillSmoothRoundRect(mx - r, my - 2, 2 * r, 4, 2, col); break;
  }
}

// --- Giọt nước mắt (SAD): tròn dưới + đỉnh nhọn trên ---
inline void drawTear(lgfx::LGFX_Sprite& c, int x, int y, int s, uint16_t col) {
  c.fillCircle(x, y, s, col);
  c.fillTriangle(x - s, y, x + s, y, x, y - s * 2, col);
}

// --- Chữ z Z Z (SLEEPY) ---
inline void drawZzz(lgfx::LGFX_Sprite& c, int x, int y, uint16_t col) {
  c.setTextColor(col);
  c.setTextSize(2); c.drawString("z", x,          y);
  c.setTextSize(3); c.drawString("Z", x + 13,     y - 15);
}
