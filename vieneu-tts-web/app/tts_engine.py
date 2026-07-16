"""Lớp bọc model VieNeu-TTS thật (gói `vieneu`).

- Nạp model một lần (lazy), dùng lại cho mọi request.
- Khoá luồng vì suy luận không an toàn khi chạy song song.
- Trả về WAV bytes để backend stream thẳng cho trình duyệt.
"""
from __future__ import annotations

import io
import os
import tempfile
import threading
from typing import Optional

from .config import settings

# Danh sách style hợp lệ theo tài liệu VieNeu-TTS
VALID_STYLES = {"tu_nhien", "tin_tuc", "doc_truyen"}


class TTSEngine:
    """Bọc `vieneu.Vieneu` với nạp lười + an toàn luồng."""

    def __init__(self) -> None:
        self._model = None
        self._lock = threading.Lock()
        os.makedirs(settings.data_dir, exist_ok=True)

    def _ensure_loaded(self):
        """Nạp model lần đầu được gọi (tải weight từ HuggingFace nếu chưa cache)."""
        if self._model is None:
            with self._lock:
                if self._model is None:
                    from vieneu import Vieneu  # import trễ để khởi động nhanh

                    self._model = Vieneu(precision=settings.precision)
        return self._model

    def list_voices(self) -> list[dict]:
        """Danh sách giọng dựng sẵn: [{label, id}]."""
        model = self._ensure_loaded()
        voices = model.list_preset_voices()
        return [{"label": label, "id": voice_id} for label, voice_id in voices]

    def _to_wav_bytes(self, model, audio) -> bytes:
        """Ghi numpy audio ra WAV bytes qua API save() của chính model (đúng sample rate)."""
        with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as tmp:
            tmp_path = tmp.name
        try:
            model.save(audio, tmp_path)
            with open(tmp_path, "rb") as f:
                return f.read()
        finally:
            try:
                os.remove(tmp_path)
            except OSError:
                pass

    def synthesize(
        self,
        text: str,
        voice: str,
        style: Optional[str] = None,
    ) -> bytes:
        """Đọc `text` bằng một giọng dựng sẵn -> WAV bytes."""
        model = self._ensure_loaded()
        with self._lock:
            kwargs = {"voice": voice}
            if style and style in VALID_STYLES:
                kwargs["style"] = style
            audio = model.infer(text, **kwargs)
            return self._to_wav_bytes(model, audio)

    def clone(
        self,
        text: str,
        ref_audio_path: str,
        denoise: bool = True,
    ) -> bytes:
        """Nhân bản giọng từ file audio mẫu (cần PyTorch) -> WAV bytes."""
        if not settings.enable_cloning:
            raise RuntimeError(
                "Tính năng nhân bản giọng đang tắt (ENABLE_CLONING=false). "
                "Cần cài PyTorch để bật."
            )
        model = self._ensure_loaded()
        with self._lock:
            audio = model.infer(text, ref_audio=ref_audio_path, denoise=denoise)
            return self._to_wav_bytes(model, audio)


# Instance dùng chung toàn app
engine = TTSEngine()
