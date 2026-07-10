# Nghiên cứu: Hardware & Software cho EMO Robot Face Animation

## So sánh 2 Nền tảng Chính

| Tiêu chí | ESP32 (+ESP32-S3) | Raspberry Pi (Zero 2W/4/5) |
|----------|---|---|
| **Chi phí** | 50-150K VN | 300-1500K VN |
| **Màn hình** | OLED (SSD1306, SH1106), TFT (GC9A01, ST7789) | DSI camera, HDMI |
| **Thư viện** | TFT_eSPI, LovyanGFX, FluxGarage RoboEyes | Pygame, Adafruit Pi_Eyes (pi3d) |
| **Ngôn ngữ** | Arduino C++, MicroPython | Python (chính) |
| **Pin** | 3-7 ngày (2000mAh) | 4-8 giờ (khi chạy video) |
| **Animation** | Sprite/GIF (giới hạn bộ nhớ), rendering procedural | Video HD, AI voice real-time |
| **Phức tạp** | Trung bình, học tập dễ | Cao, dev nhanh |

## Thư viện Open-Source Sẵn Có

**ESP32-centric:**
- **FluxGarage RoboEyes** (Arduino): OLED, cảm xúc (happy/laughing/confused), autoblink, idle mode. [GitHub](https://github.com/FluxGarage/RoboEyes)
- **esp32-eyes**: Emotive OLED, Anki Cozmo-style. [GitHub](https://github.com/playfultechnology/esp32-eyes)
- **ESP32-Pet-Robot**: Touch/sound responsive, 7 emotions (happy/angry/sleepy/scared). [GitHub](https://github.com/SukunDev/ESP32-Pet-Robot)
- **RoboFace**: Wrapper cho SH1106 OLED. [PlatformIO](https://registry.platformio.org/libraries/kingsmen732/RoboFace)
- **TFT_eSPI**, **LovyanGFX**: Thư viện rendering TFT tổng quát

**Raspberry Pi:**
- **Adafruit Pi_Eyes**: pi3d library, frame-by-frame animation (không video). [GitHub](https://github.com/adafruit/Pi_Eyes)
- **Pygame**: Python animation/sprite engine, dùng cho robot face GIF/sprite

## Phần Cứng Cụ Thể & Giá VN

**ESP32 Bundle:**
- ESP32-S3 + GC9A01 round display (240x240): ~200-300K (Shopee, Icdayroi, Hshop.vn)
- Pin lithium 2000-3000mAh + TP4056 charger: ~80-150K
- Vỏ in 3D: miễn phí (Thingiverse)

**Raspberry Pi Bundle:**
- Pi Zero 2W: ~400-500K
- Camera DSI OV5647 / IMX219: ~300-400K
- Touchscreen 3.5" DSI: ~800-1200K
- Pin + boost circuit (1A 5V): ~200K

## Kiến Trúc State Machine (Cảm Xúc)

```
[Idle] <-> [Blink]
  ↓        (random, 200ms)
[Happy] -> [Sad] -> [Angry]
  ↑         ↓        ↓
[Sleepy] <- -> [Confused]
```

- **Blink**: Automatic mỗi 3-5 giây, duration 150-200ms
- **Transitions**: Easing (ease-out 500ms) giữa trạng thái
- **Animation**: Sprite-sheet hoặc draw procedural (circle + bezier cho mắt)
- **Timing**: Non-blocking event loop (async-friendly)

## Trade-off Analysis

**ESP32 Thắng:** Giá rẻ (1/3 Pi), pin lâu, tích hợp WiFi/BLE, deploy đơn giản (upload code)
**ESP32 Thua:** Bộ nhớ hạn chế (~520KB RAM), animation 2D đơn, khó thêm AI voice

**Pi Thắng:** Mạnh (quad-core), chạy video HD, Python dev nhanh, AI voice + NLP khả thi
**Pi Thua:** Tốn pin (5V+), đắt, phức tạp boot sequence, cần cooling

## Khuyến nghị

1. **Bắt đầu với ESP32-S3 + GC9A01 round display**
   - Mở ngoài sớm, battery life tốt, FluxGarage/RoboEyes mature
   - Sau này upgrade Pi nếu cần AI voice

2. **Code architecture**: State machine + async event loop → dễ scale
3. **Graphics**: SVG → PNG sprite-sheet. Tool: Inkscape + ImageMagick
4. **Testing**: Wokwi (ESP32 emulator online) trước hardware

## Câu hỏi Chưa Giải Đáp

- Giá chính xác trên Shopee VN hiện tại (hardware thay đổi liên tục)?
- Dual round eyes (2x GC9A01) vs single large display?
- Motion control (head servo) integrate thế nào?
- Audio streaming (bluetooth speaker) timing sync?

---

**Sources:**
- [FluxGarage RoboEyes](https://github.com/FluxGarage/RoboEyes)
- [esp32-eyes](https://github.com/playfultechnology/esp32-eyes)
- [ESP32-Pet-Robot](https://github.com/SukunDev/ESP32-Pet-Robot)
- [Adafruit Pi_Eyes](https://github.com/adafruit/Pi_Eyes)
- [GC9A01 on Spotpear](https://spotpear.com/)
- [DroneBot Workshop - GC9A01](https://dronebotworkshop.com/gc9a01/)
