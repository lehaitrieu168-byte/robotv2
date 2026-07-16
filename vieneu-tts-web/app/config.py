"""Cấu hình ứng dụng đọc từ biến môi trường (.env).

Giữ mọi tham số vận hành ở một chỗ để dễ chỉnh khi self-host.
"""
from __future__ import annotations

import os
from dataclasses import dataclass

try:
    from dotenv import load_dotenv

    load_dotenv()  # nạp biến từ file .env (nếu có) — vd GROQ_API_KEY
except ImportError:
    pass


def _get_bool(name: str, default: bool) -> bool:
    val = os.getenv(name)
    if val is None:
        return default
    return val.strip().lower() in {"1", "true", "yes", "on"}


@dataclass(frozen=True)
class Settings:
    # Máy chủ
    host: str = os.getenv("HOST", "0.0.0.0")
    port: int = int(os.getenv("PORT", "8000"))

    # Model VieNeu-TTS
    # precision: "int8" (mặc định, nhanh trên CPU) hoặc "fp32" (chất lượng cao nhất)
    precision: str = os.getenv("VIENEU_PRECISION", "int8")

    # Bật/tắt tính năng nhân bản giọng (voice cloning).
    # Cloning cần PyTorch được cài; nếu chỉ chạy ONNX (không torch) thì tắt đi.
    enable_cloning: bool = _get_bool("ENABLE_CLONING", True)

    # Nạp sẵn model lúc khởi động (nên bật khi demo với robot để request đầu không timeout)
    preload_model: bool = _get_bool("PRELOAD_MODEL", False)

    # ---- Bộ não trò chuyện (Groq) ----
    groq_api_key: str = os.getenv("GROQ_API_KEY", "")
    groq_stt_model: str = os.getenv("GROQ_STT_MODEL", "whisper-large-v3-turbo")
    groq_llm_model: str = os.getenv("GROQ_LLM_MODEL", "llama-3.3-70b-versatile")
    chat_system_prompt: str = os.getenv(
        "CHAT_SYSTEM_PROMPT",
        "Bạn tên là Vivi, một robot nhỏ dễ thương, trò chuyện thân thiện, vui vẻ, hài hước nhẹ. "
        "Khi được chào thì tự giới thiệu mình là Vivi. "
        "LUÔN trả lời NGẮN GỌN bằng tiếng Việt, tối đa 1-2 câu, giọng gần gũi. "
        "Không dùng emoji, không markdown, chỉ nói như đang trò chuyện.",
    )

    # Giới hạn để tránh lạm dụng khi mở ra mạng
    max_chars: int = int(os.getenv("MAX_CHARS", "1000"))
    max_ref_seconds: int = int(os.getenv("MAX_REF_SECONDS", "30"))

    # Thư mục lưu voice đã lưu (add_voice) và cache tạm
    data_dir: str = os.getenv("DATA_DIR", "data")


settings = Settings()
