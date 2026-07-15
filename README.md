# Robot EMO — màn hình biểu cảm + loa + audio từ điện thoại

Robot để bàn kiểu [EMO (Living.AI)](https://living.ai/emo/): một màn hình hiển thị hai con mắt biết chớp và đổi cảm xúc, loa phát âm thanh, nhận giọng nói từ điện thoại qua Bluetooth, và báo thức.

Không servo, không bánh xe. Vỏ in 3D đã có sẵn.

**Trạng thái:** đang lập kế hoạch. Chưa có code.

## Phần cứng

Board all-in-one **ESP32 "Cheap Yellow Display" (CYD, ESP32-2432S028R)**:

- ESP32-WROOM-32 — có Bluetooth Classic, cần thiết để làm loa A2DP
- Màn hình ILI9341 2.8" 320×240 (SPI)
- Cảm ứng điện trở XPT2046
- Quang trở (LDR) onboard
- Amp SC8002B + cổng loa JST

### Danh sách mua

| Món | Ước tính (VND) | Bắt buộc |
|---|---|---|
| CYD ESP32-2432S028R | 200–300k | ✅ |
| Loa nhỏ + giắc JST 1.25 2-pin | 25–40k | ✅ |
| Cáp micro-USB **truyền dữ liệu** | 0–30k | ✅ |
| Vỏ 3D | 0đ (đã in) | ✅ |
| **Tổng** | **~225–370k** | |

Giá ước tính, cần kiểm tra lại trên Shopee / Hshop.vn / Icdayroi.

Báo thức không tốn thêm đồng nào (lấy giờ qua NTP, dùng RTC nội của chip).

## Ràng buộc kỹ thuật

Ba điều đã xác minh từ [PINS.md của witnessmenow](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/PINS.md), quyết định toàn bộ thiết kế:

**CYD chỉ lộ ra 3 chân dùng được** — IO35 (chỉ đọc), IO22, IO27 — qua giắc Molex PicoBlade 1.25mm, không phải hàng rào 2.54mm. I2S cần 3 chân xuất tín hiệu, nên **không gắn được amp ngoài nếu không hàn**. Âm thanh v1 đi qua loa onboard với DAC 8-bit tích hợp.

**DAC nội nằm ở GPIO25 và GPIO26, mà GPIO25 là chân xung của cảm ứng.** Bật DAC hai kênh sẽ làm chết cảm ứng màn hình. Bắt buộc cấu hình mono, chỉ dùng GPIO26.

**Wi-Fi và Bluetooth Classic giành nhau sóng 2.4GHz.** Lấy giờ NTP một lần lúc khởi động, tắt Wi-Fi, rồi mới bật A2DP. Chip tự giữ giờ, trôi khoảng 1–2 giây mỗi ngày.

## Kế hoạch

Chi tiết trong [`plans/260710-0916-emo-robot-face-audio-streaming/`](plans/260710-0916-emo-robot-face-audio-streaming/plan.md) — 9 phase, ~35 giờ phần lõi.

| # | Phase | Effort |
|---|---|---|
| 01 | Chuẩn bị: kiểm chứng rủi ro, đo vỏ 3D, mua đồ, cài IDE | 4h |
| 02 | Màn hình + hai mắt biểu cảm (TFT_eSPI, sprite) | 6h |
| 03 | Âm thanh qua loa onboard (DAC nội) | 4h |
| 04 | Bluetooth A2DP sink — nhận audio từ điện thoại | 3h |
| 05 | Lip-sync, phản ứng theo âm lượng, chia core FreeRTOS | 5h |
| 06 | Tương tác: cảm ứng + quang trở | 3h |
| 07 | Lắp vào vỏ 3D | 5h |
| 08 | *(Dự phòng)* Wi-Fi + PWA nếu A2DP thất bại | 8h |
| 09 | Báo thức: giờ NTP, nhiều báo thức, mắt "bừng tỉnh", giọng nói nhắc nhở (tự thu, phát ngẫu nhiên) | 6h |

## Rủi ro đã biết

**Chất âm.** DAC nội chỉ 8-bit — nghe rõ giọng nói, nhưng nhạc có nhiễu nền. Muốn 16-bit phải hàn dây lấy chân cho amp I2S ngoài, hoặc đổi sang board rời.

**Bluetooth trên iOS.** Cách dùng dự kiến là chạy app mic-to-speaker trên điện thoại rồi route sang robot như một loa Bluetooth. Android thường được; iOS siết chặt routing nên có thể không chạy. Phase 01 bắt kiểm chứng bằng loa Bluetooth thường **trước khi mua bất cứ thứ gì**; nếu hỏng thì chuyển sang Phase 08.

**Wi-Fi 5GHz.** ESP32 không bắt được băng 5GHz. Router hai băng phải tách tên mạng 2.4GHz riêng.

## Ghi chú

Người thực hiện mới bắt đầu với điện tử, chưa hàn bao giờ. Toàn bộ thiết kế v1 chọn phương án không cần hàn.
