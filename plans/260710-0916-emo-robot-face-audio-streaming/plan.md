---
title: "Robot EMO: màn hình biểu cảm + loa + nhận audio từ điện thoại"
description: "Robot desktop trên board CYD (ESP32-2432S028R): 2 mắt biểu cảm, loa onboard (DAC nội 8-bit), nhận giọng từ điện thoại qua Bluetooth A2DP. Ngân sách ≤ 600k, không hàn."
status: pending
priority: P2
effort: 35h core (P1-P7, P9) + 8h optional (P8)
branch: main
tags: [esp32, cheap-yellow-display, hardware, robotics, bluetooth-a2dp, tft-display]
created: 2026-07-10
---

# Robot EMO — Implementation Plan

Robot desktop kiểu EMO (Living.AI): 1 màn TFT hiển thị 2 mắt biểu cảm + loa phát âm thanh + nhận giọng từ điện thoại (Bluetooth A2DP sink). KHÔNG servo, KHÔNG bánh xe. Người làm MỚI BẮT ĐẦU, **CHƯA HÀN BAO GIỜ** → v1 dùng board all-in-one, không hàn, không nối dây tín hiệu ngoài. **Ngân sách ≤ 600.000 VND.** v1 chạy nguồn USB 5V (0đ). **Vỏ 3D user đã in sẵn (0đ).**

## Kiến trúc đã chốt (v1)
- Board: **ESP32 "Cheap Yellow Display" (CYD, ESP32-2432S028R)** — ESP32-WROOM-32 (có BT Classic → A2DP ✅) + màn ILI9341 2.8" 320x240 + cảm ứng điện trở XPT2046 + LDR (IO34) + khe SD. **KHÔNG PSRAM.**
- Audio OUT: **loa onboard cắm cổng JST "SPEAK"** → amp SC8002B → **DAC nội 8-bit (GPIO26), cấu hình MONO chỉ kênh GPIO26**. KHÔNG dùng MAX98357A/I2S ở v1 (xem Risk).
- ⚠️ **DAC1=GPIO25 = CLK cảm ứng XPT2046.** Nếu bật DAC 2 kênh → cảm ứng chết. BẮT BUỘC chỉ bật kênh GPIO26 (ghi rõ ở P3/P4/P6).
- Audio IN: điện thoại → app mic-to-speaker → robot = loa Bluetooth (A2DP sink `pschatzmann/ESP32-A2DP`, xuất `AnalogAudioStream`/DAC nội). Tắt Wi-Fi khi A2DP.
- Màn/mắt: TFT_eSPI + User_Setup CYD. WROOM không PSRAM (~160KB heap sau BT) → sprite full 320x240=150KB KHÔNG khả thi → **2 sprite nhỏ mỗi mắt (~20KB/mắt)** (điểm kỹ thuật khó nhất — P2).
- Tương tác: cảm ứng XPT2046 + LDR onboard → 0 chân thêm, 0đ. HC-SR04 LOẠI (cổng CYD là PicoBlade 1.25mm, cần dây JST riêng + chỉ còn IO22/IO27/IO35; cảm ứng+LDR đã đủ — YAGNI).
- Concurrency: core 0 = BT/audio, core 1 = render TFT (FreeRTOS pinned task).

## ⚠️ Risk lớn nhất (đọc trước)
1. **Kỳ vọng chất âm vs phần cứng:** user muốn "nghe nhạc tử tế" nhưng **DAC nội CYD chỉ 8-bit → nhiễu nền, không hi-fi**. Đủ cho giọng nói + nhạc nhẹ. Đạt 16-bit **phải HÀN** (MAX98357A) — nằm ngoài v1. Đường nâng cấp ở cuối P3. Khuyến nghị: làm v1, nghe thử, rồi quyết.
2. **A2DP mic-to-speaker có thể FAIL trên iOS** (routing hạn chế) → verify bằng loa BT ở P1; fallback P8 (Wi-Fi/PWA).
3. **ESP32 CHỈ bắt Wi-Fi 2.4GHz, KHÔNG bắt 5GHz** — cần để sync giờ NTP cho báo thức (P9). Router 2 băng tần phải bật/tách SSID 2.4GHz (bẫy phổ biến).
> Ghi chú: `researcher-03` report có SAI SÓT — pinout (IO26 không lộ ra), giá CYD 380-450k thổi phồng (thực ~200-300k), "đủ GPIO cho HC-SR04" sai. Đã đính chính theo PINS.md witnessmenow.

## Phases
| # | Phase | File | Status | Effort |
|---|-------|------|--------|--------|
| 01 | Chuẩn bị: verify A2DP (loa BT), đo vừa vỏ 3D, hỏi shop kèm loa/dây JST, cài IDE | [phase-01](./phase-01-chuan-bi-verify-a2dp-mua-linh-kien-cai-ide.md) | pending | 4h |
| 02 | Màn CYD + 2 mắt biểu cảm (TFT_eSPI, 2 sprite mắt, state machine cảm xúc) | [phase-02](./phase-02-man-hinh-tft-mat-bieu-cam-sprite.md) | pending | 6h |
| 03 | Âm thanh loa onboard + DAC nội (mono GPIO26), phát WAV từ LittleFS | [phase-03](./phase-03-am-thanh-loa-onboard-dac-noi-phat-wav.md) | pending | 3h |
| 04 | Bluetooth A2DP sink → DAC nội; app mic-to-speaker | [phase-04](./phase-04-bluetooth-a2dp-sink-nhan-audio-dien-thoai.md) | pending | 3h |
| 05 | Lip-sync / phản ứng âm lượng (RMS từ A2DP → mắt), chia core FreeRTOS | [phase-05](./phase-05-lip-sync-phan-ung-am-luong-freertos.md) | pending | 5h |
| 06 | Tương tác: cảm ứng XPT2046 + LDR onboard → trigger cảm xúc | [phase-06](./phase-06-tuong-tac-cam-ung-xpt2046-quang-tro-ldr.md) | pending | 3h |
| 07 | Lắp vào vỏ 3D đã in: cố định CYD, gắn loa, chống chạm chập (nguồn USB) | [phase-07](./phase-07-lap-rap-vao-vo-3d-da-in.md) | pending | 4h |
| 08 | (Optional) Fallback Wi-Fi UDP/WebSocket + PWA nếu A2DP mic-through fail | [phase-08](./phase-08-optional-fallback-wifi-udp-websocket-pwa.md) | pending | 8h |
| 09 | Báo thức: giờ NTP (sync boot) → **nhiều** báo thức + mắt "wake" + **giọng nói user thu (phát ngẫu nhiên)**; đặt/xoá bằng cảm ứng | [phase-09](./phase-09-bao-thuc-dong-ho-ntp.md) | pending | 6h |

## Dependencies
- **P1 chặn tất cả.** Cổng quyết định 1: verify A2DP → FAIL thì P8 thay P4. Cổng quyết định 2: CYD vừa vỏ 3D không.
- P2, P3, P4 độc lập nhưng người mới nên tuần tự. **P5 cần P2+P4. P6 cần P2. P7 cần P2..P6. P8 chỉ khi P1 fail. P9 (báo thức) cần P2+P3+P6.**

## BOM v1 (VND, 2026) — cần verify Shopee/Hshop/Icdayroi
> Báo thức (P9) = **0đ thêm** (dùng NTP qua Wi-Fi + RTC nội ESP32). Không mua gì.
| Món | SL | Ước tính | Bắt buộc? |
|-----|----|---------|-----------|
| CYD ESP32-2432S028R (bản cảm ứng điện trở) | 1 | 200-300k | ✅ |
| Loa nhỏ + giắc JST 1.25 2-pin (nhiều shop kèm CYD — hỏi trước; SC8002B ~3W/4Ω, verify) | 1 | 25-40k | ✅ |
| Cáp micro-USB **truyền dữ liệu** (không phải cáp chỉ sạc) | 1 | 0-30k | ✅ |
| Vỏ 3D | 1 | **0đ (đã in)** | ✅ |
| **CỘNG BẮT BUỘC** | | **~225-370k** | **≤ 600k ✅** |
| Đồng hồ vạn năng (đo nguồn trước khi cắm) | 1 | 80-150k | ⬜ nên có |
| Dây JST 1.25 (PicoBlade) 4-pin | 2 | 15-30k | ⬜ chỉ khi mở rộng IO22/IO27 |
| MAX98357A + loa 3W (**nâng cấp audio 16-bit, CẦN HÀN**) | 1 | 75-110k | ⬜ tuỳ chọn |
| Pin 18650 + đế + TP4056 + boost MT3608 | 1 | 80-110k | ⬜ tuỳ chọn (v1 chạy USB) |
| DS3231 RTC (I2C: IO22 SDA/IO27 SCL/3.3V/GND trên CN1) + dây JST 1.25 4-pin | 1 | 45-80k | ⬜ tuỳ chọn — giữ giờ khi mất điện/không Wi-Fi (P9) |

## Câu hỏi chưa giải đáp (toàn dự án)
1. Điện thoại user **iOS hay Android**? → rủi ro A2DP mic-through. Verify P1.
2. **Kích thước khoang vỏ 3D** vs CYD (~86×50mm)? Có sẵn lỗ loa/ô LDR/khe USB chưa?
3. Shop bán CYD có tặng kèm loa + dây JST 1.25 không? Loa mấy W/Ω?
4. Chiều kênh DAC LEFT/RIGHT ↔ GPIO25/26 (test chạm màn khi phát nhạc)?
5. **Wi-Fi 2.4GHz** cho NTP (P9)? (ESP32 không bắt 5GHz.) ⟵ cần trước khi làm báo thức.
> P9 đã CHỐT với user: **nhiều báo thức (tối đa 4), giọng nói tự thu phát ngẫu nhiên** (không phải chuông). Chi tiết trong phase-09.
