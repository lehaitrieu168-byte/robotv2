# VieNeu-TTS Web — Text-to-Speech tiếng Việt self-host

Trang web tự host để chuyển văn bản tiếng Việt thành giọng nói, có **nhân bản giọng
(voice cloning)** từ audio mẫu 3–8 giây. Chạy hoàn toàn trên máy bạn bằng model
[VieNeu-TTS](https://github.com/pnnbao97/VieNeu-TTS) (48kHz, offline).

Backend FastAPI bọc model thật + giao diện web tối giản. Không cần API key, không gửi
dữ liệu ra ngoài.

## Tính năng

- 🔊 Đọc văn bản bằng **giọng dựng sẵn** (chọn từ danh sách của model)
- 🧬 **Nhân bản giọng** từ file audio mẫu (WAV/MP3, 3–8s) — cần PyTorch
- 🎭 Phong cách đọc: tự nhiên / bản tin / đọc truyện
- 😄 Chèn biểu cảm: `[cười]`, `[thở dài]`, `[hắng giọng]`
- ⬇️ Nghe trực tiếp + tải file WAV
- 🐳 Đóng gói Docker, self-host một lệnh

## Chạy nhanh bằng Docker (khuyến nghị)

```bash
cd vieneu-tts-web
docker compose up --build
```

Mở trình duyệt: <http://localhost:8000>

> Lần đầu chạy sẽ **tải model từ HuggingFace** (vài trăm MB) nên hơi lâu; các lần sau
> dùng cache trong volume `vieneu-models` nên nhanh.

Chỉ cần giọng dựng sẵn (image nhẹ hơn, không cài PyTorch)? Sửa `docker-compose.yml`:
`ENABLE_CLONING: "false"`.

## Chạy trực tiếp (không Docker)

Yêu cầu Python ≥ 3.10.

```bash
cd vieneu-tts-web
python -m venv .venv
# Windows: .venv\Scripts\activate | Linux/macOS: source .venv/bin/activate
pip install -r requirements.txt

# (Tùy chọn) bật nhân bản giọng — cần PyTorch:
pip install torch torchaudio --index-url https://download.pytorch.org/whl/cpu

python -m app.main
```

Mở <http://localhost:8000>.

## Cấu hình (biến môi trường)

Sao chép `.env.example` → `.env` và chỉnh. Xem bảng:

| Biến | Mặc định | Ý nghĩa |
|---|---|---|
| `PORT` | `8000` | Cổng web |
| `VIENEU_PRECISION` | `int8` | `int8` (nhanh) hoặc `fp32` (chất lượng cao) |
| `ENABLE_CLONING` | `true` | Bật nhân bản giọng (cần PyTorch) |
| `MAX_CHARS` | `1000` | Giới hạn độ dài văn bản mỗi lần |
| `HF_HOME` | `/models` | Nơi cache model HuggingFace |

## API

| Method | Endpoint | Mô tả |
|---|---|---|
| GET | `/api/health` | Trạng thái + cấu hình |
| GET | `/api/voices` | Danh sách giọng dựng sẵn |
| POST | `/api/tts` | Form: `text`, `voice`, `style` → trả WAV |
| POST | `/api/clone` | Form: `text`, `ref_audio` (file), `denoise` → trả WAV |

Ví dụ:

```bash
curl -X POST http://localhost:8000/api/tts \
  -F "text=Xin chào Việt Nam" -F "voice=<voice_id>" --output out.wav
```

## Cấu trúc

```
vieneu-tts-web/
├── app/
│   ├── config.py        # đọc cấu hình từ env
│   ├── tts_engine.py    # bọc model VieNeu-TTS (nạp lười, an toàn luồng)
│   ├── routes.py        # các endpoint API
│   ├── main.py          # khởi tạo FastAPI + mount frontend
│   └── static/          # giao diện web (index.html, style.css, app.js)
├── Dockerfile
├── docker-compose.yml
├── requirements.txt
└── .env.example
```

## GPU (tùy chọn, tăng tốc)

Trên máy có CUDA, cài PyTorch bản CUDA trước khi cài `vieneu`; SDK tự chuyển sang engine
PyTorch. Xem hướng dẫn tại [repo VieNeu-TTS](https://github.com/pnnbao97/VieNeu-TTS).

## Lưu ý bảo mật

- Không đặt API key / secret vào repo. `.env` đã được `.gitignore`.
- Mặc định server mở ở `0.0.0.0:8000`. Khi đưa ra Internet nên đặt sau reverse proxy
  (Nginx/Caddy) + HTTPS và cân nhắc giới hạn truy cập.

## Nguồn

Model & trọng số: [pnnbao97/VieNeu-TTS](https://github.com/pnnbao97/VieNeu-TTS) ·
[pnnbao-ump/VieNeu-TTS trên HuggingFace](https://huggingface.co/pnnbao-ump/VieNeu-TTS).
Dự án này chỉ là lớp web self-host bọc quanh model đó.
