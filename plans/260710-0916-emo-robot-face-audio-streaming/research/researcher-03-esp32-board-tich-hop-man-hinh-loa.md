# Xác minh Board ESP32 Tích hợp Màn hình + Loa
**Mục tiêu:** Robot chạy A2DP sink (loa Bluetooth) + animation mắt, ngân sách ≤600k VND, người mới.

---

## Bảng So Sánh Ứng Viên

| Tiêu chí | **CYD (2432S028R)** | **M5Stack Core2** | **M5Stack Basic** |
|---------|-------------------|------------------|------------------|
| **Chip** | ESP32-WROOM-32 | ESP32-D0WDQ6-V3 | ESP32-D0WDQ6-V3 |
| **BT Classic** | ✅ | ✅ | ✅ |
| **Màn hình** | 2.8" ILI9341 320×240 | 2.0" capacitive | 2.0" IPS |
| **Audio path** | SC8002B → DAC 8-bit (GPIO26) | I2S → NS4168 amp | I2S (dự kiến) |
| **Audio quality** | Kém (8-bit) | Tốt (I2S) | Tốt (I2S) |
| **PSRAM** | ❌ | 8MB | 8MB |
| **RAM tự do** | ~160KB (sau Bluetooth) | ~2MB | ~2MB |
| **Touchscreen** | XPT2046 (resistive) | Capacitive | ❌ |
| **Loa kèm** | ? (cần verify) | ? | ? |
| **Giá VN** | ~380-450k VND | ~800k+ VND | ~600-700k VND |
| **Phù hợp** | ✅ **CHỌN** | ❌ Vượt ngân sách | ❌ Vượt ngân sách |

---

## Phân Tích Chi Tiết

### **CYD (ESP32-2432S028R)** — ĐỀ XUẤT CHÍNH
**Ưu điểm:**
- Chip ESP32-WROOM-32 native Bluetooth Classic (A2DP sink) ✅
- **Cực rẻ** (~380k VND) → dư budget cho cảm biến/loa phụ
- All-in-one: màn hình 2.8" TFT + touchscreen + loa amp + SD card
- Pinout phong phú (GPIO25, GPIO26 cho DAC; GPIO34-39 input-only cho HC-SR04)
- Thư viện `pschatzmann/ESP32-A2DP` hỗ trợ xuất internal DAC trực tiếp (AnalogAudioStream)

**Hạn chế:**
- **DAC 8-bit nội**: chất lượng audio kém, nhiều noise, không phù hợp nhạc chất lượng cao nhưng **OK cho giọng nói/nhạc nhẹ**
- RAM ~160KB sau Bluetooth stack: **động lực cho tối ưu vẽ mắt** (sprite, không full-screen, tile-based rendering)
- Không PSRAM → không cache toàn bộ hình ảnh
- SPI dùng chung TFT + SD: cần chia core FreeRTOS, có thể gây rè nhẹ nếu không cẩn thận

**Audio Implementation:**
```cpp
#include "AudioTools.h"
#include "BluetoothA2DPSink.h"

AnalogAudioStream out;  // DAC GPIO25/26 tự động
BluetoothA2DPSink a2dp_sink(out);

void setup() {
  a2dp_sink.start("Robot");  // Tên BT hiển thị
}
```
Loa nối vào SPEAK connector (JST 1.25mm) qua SC8002B amp.

---

### **M5Stack Core2 / Basic** — Loại vì ngân sách
- Chip OK (ESP32 classic), audio I2S chất lượng tốt (NS4168), PSRAM 8MB ✅
- **Nhưng giá VN ~800k+ (Core2) / 600-700k (Basic)**: gần/vượt 600k → không đủ room cho linh kiện phụ
- Màn hình nhỏ (2.0") → animation mắt sẽ nhỏ hơn CYD

---

### **LilyGO T-Display / T-Embed / T-Deck**
- Thông tin không đủ từ search. Cần research thêm nếu muốn fallback.

---

## Vấn Đề Kỹ Thuật

### **SPI + Bluetooth rè/giật?**
- TFT dùng SPI (GPIO23/19 MOSI/MISO, GPIO18 CLK, GPIO5 CS)
- A2DP chạy RTOS khác core → ít gây rè **nếu TFT refresh ≤30 FPS**
- Khuyến nghị: Backlight PWM riêng (GPIO2), SPI clock ≤40MHz

### **RAM cho animation mắt?**
- WROOM-32 = 320KB heap, sau Bluetooth ~160KB trống
- **Đủ cho animation pixel-art** (ví dụ: 2-3 frame sprite 32×32 = 3KB)
- Không đủ để buffer toàn màn hình (240×320×16-bit = 150KB)
- Cách tối ưu: vẽ trực tiếp từ bộ nhớ chương trình (PROGMEM) hoặc SD card (chậm)

### **Backlight brightness?**
- GPIO27 PWM điều khiển độ sáng, default 80%
- Có thể giảm để tiết kiệm điện

---

## Khuyến nghị

### **Lựa chọn: ESP32-2432S028R (CYD)**

**Lý do:**
1. **Giá tối ưu** (~380-450k VND) ✅ dưới ngân sách, còn ~150-200k cho cảm biến/loa/dây cáp
2. **Bluetooth Classic + A2DP** natively hỗ trợ qua pschatzmann/ESP32-A2DP
3. **All-in-one** (màn hình + loa amp + touchscreen + SD card)
4. **Đủ GPIO** cho HC-SR04 hoặc cảm biến khác
5. **Người mới friendly** (thư viện phong phú, tutorial nhiều, Github/Arduino community lớn)
6. **Audio quality**: DAC 8-bit OK cho lời nói (chính yêu cầu), không ideal cho nhạc stereo

**Setup phỏng chừng:**
- Board CYD: 380k
- Loa 2W JST: 50k (hoặc dùng cái kèm)
- Cảm biến HC-SR04: 30k
- Cáp/dây: 30k
- **Tổng: ~490k VND** ✅

---

## Câu hỏi Chưa Giải Đáp

1. **CYD có loa kèm theo hay mua rời?** (Cần verify retailer)
2. **Loa kèm bao nhiêu watt, trở kháng?** (SC8002B datasheet chưa tìm được)
3. **Giá chính xác Shopee/Hshop VN 2026?** (Search không trả về)
4. **M5Stack Basic giá chính xác VN?** (Không tìm được official VN distributor)
5. **DAC 8-bit + A2DP có xảy ra aliasing khi stream 16-bit MP3/AAC?** (Lý thuyết: có, nhưng ít nghe thấy với voice)
6. **SPI TFT + A2DP concurrent có gây rè/popping?** (Chưa test thực)
7. **GPIO nào trên CYD còn trống để cắm HC-SR04?** (Cần xem full pinout PINS.md)

---

## Tài Liệu Tham Khảo

- [ESP32-2432S028R Pinout & Specs (espboards.dev)](https://www.espboards.dev/esp32/cyd-esp32-2432s028/)
- [Mischianti CYD Pinout](https://mischianti.org/esp32-2432s028-cheap-yellow-display-high-resolution-pinout-datasheet-schema-and-specs/)
- [pschatzmann/ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP)
- [ESP32-A2DP Audio Output Wiki](https://github.com/pschatzmann/ESP32-A2DP/wiki/Audio-Output)
- [Random Nerd Tutorials - CYD](https://randomnerdtutorials.com/cheap-yellow-display-esp32-2432s028r/)
- [witnessmenow/ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
- [M5Stack Core2 Docs](https://docs.m5stack.com/en/core/core2)

