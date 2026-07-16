"""Cấu hình ứng dụng đọc từ biến môi trường (.env).

Giữ mọi tham số vận hành ở một chỗ để dễ chỉnh khi self-host.
"""
from __future__ import annotations

import os
from dataclasses import dataclass


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

    # Giới hạn để tránh lạm dụng khi mở ra mạng
    max_chars: int = int(os.getenv("MAX_CHARS", "1000"))
    max_ref_seconds: int = int(os.getenv("MAX_REF_SECONDS", "30"))

    # Thư mục lưu voice đã lưu (add_voice) và cache tạm
    data_dir: str = os.getenv("DATA_DIR", "data")


settings = Settings()
