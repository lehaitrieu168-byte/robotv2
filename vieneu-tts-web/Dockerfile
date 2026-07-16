# VieNeu-TTS Web — image self-host (CPU, có sẵn voice cloning qua PyTorch CPU)
FROM python:3.11-slim

# Bật voice cloning (cài PyTorch CPU). Đặt "false" để image nhẹ hơn, chỉ dùng giọng dựng sẵn.
ARG ENABLE_CLONING=true

ENV PYTHONUNBUFFERED=1 \
    PIP_NO_CACHE_DIR=1 \
    HF_HOME=/models

# Thư viện hệ thống cho xử lý audio
RUN apt-get update && apt-get install -y --no-install-recommends \
    ffmpeg libsndfile1 espeak-ng \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY requirements.txt .
RUN pip install --upgrade pip && pip install -r requirements.txt

# PyTorch CPU cho tính năng nhân bản giọng (chỉ cài khi ENABLE_CLONING=true)
RUN if [ "$ENABLE_CLONING" = "true" ]; then \
      pip install torch torchaudio --index-url https://download.pytorch.org/whl/cpu ; \
    fi

COPY app ./app

EXPOSE 8000

# Cache model HuggingFace vào volume /models để không tải lại mỗi lần khởi động
VOLUME ["/models"]

CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000"]
