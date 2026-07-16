#pragma once
// Bảng thông số cho từng cảm xúc — 1 NƠI DUY NHẤT để chỉnh mắt/mí/miệng/hiệu ứng.
// Thêm cảm xúc mới = thêm 1 giá trị vào enum Mood + 1 dòng trong moodStyle().
#include <stdint.h>

enum Mood {
  MOOD_NEUTRAL, MOOD_HAPPY, MOOD_ANGRY, MOOD_TIRED, MOOD_SURPRISED,
  MOOD_LOVE, MOOD_SAD, MOOD_SLEEPY,
  MOOD_COUNT                          // luôn để cuối: đếm số cảm xúc
};

// Kiểu mí / lông mày phủ lên mắt (khoét bằng màu nền)
enum Lid   { LID_NONE, LID_ANGRY, LID_TIRED, LID_SAD, LID_HAPPY };
// Kiểu miệng
enum Mouth { MOUTH_LINE, MOUTH_SMILE, MOUTH_FROWN, MOUTH_FLAT, MOUTH_OPEN };

struct MoodStyle {
  float wScale, hScale;   // hệ số kích thước mắt so với kích thước gốc
  Lid   lid;              // mí / lông mày
  Mouth mouth;            // kiểu miệng
  bool  heart;            // LOVE: thay mắt bằng trái tim
  bool  closed;           // SLEEPY: mắt nhắm (đường cong nằm ngang)
  bool  tear;             // SAD: thêm giọt nước mắt rơi
  bool  zzz;              // SLEEPY: hiện chữ z Z Z bay lên
  float shake;            // biên độ rung ngang khi chuyển cảnh (px)
  float pop;              // độ nảy squash & stretch khi chuyển cảnh
};

// Lấy bộ thông số của 1 cảm xúc. Chỉnh "tính cách" robot ngay tại đây.
inline MoodStyle moodStyle(Mood m) {
  switch (m) {
    //                     wS     hS    lid        mouth        heart  closed tear   zzz    shake  pop
    case MOOD_HAPPY:     return {1.00f, 1.00f, LID_HAPPY, MOUTH_SMILE, false, false, false, false, 0.f,  0.12f};
    case MOOD_ANGRY:     return {1.00f, 1.00f, LID_ANGRY, MOUTH_FROWN, false, false, false, false, 6.f,  0.10f};
    case MOOD_TIRED:     return {1.00f, 0.85f, LID_TIRED, MOUTH_FLAT,  false, false, false, false, 0.f,  0.06f};
    case MOOD_SURPRISED: return {1.25f, 1.15f, LID_NONE,  MOUTH_OPEN,  false, false, false, false, 0.f,  0.24f};
    case MOOD_LOVE:      return {1.00f, 1.00f, LID_NONE,  MOUTH_SMILE, true,  false, false, false, 0.f,  0.16f};
    case MOOD_SAD:       return {0.95f, 0.90f, LID_SAD,   MOUTH_FROWN, false, false, true,  false, 0.f,  0.06f};
    case MOOD_SLEEPY:    return {1.00f, 0.85f, LID_NONE,  MOUTH_FLAT,  false, true,  false, true,  0.f,  0.05f};
    default:             return {1.00f, 1.00f, LID_NONE,  MOUTH_LINE,  false, false, false, false, 0.f,  0.08f};
  }
}
