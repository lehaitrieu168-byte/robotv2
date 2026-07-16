"""Các endpoint API cho web TTS.

- GET  /api/health        -> trạng thái + cấu hình
- GET  /api/voices        -> danh sách giọng dựng sẵn
- POST /api/tts           -> đọc text bằng giọng dựng sẵn (WAV)
- POST /api/clone         -> nhân bản giọng từ audio mẫu (WAV)
"""
from __future__ import annotations

import os
import tempfile
from collections import deque
from urllib.parse import quote

from fastapi import APIRouter, File, Form, HTTPException, UploadFile
from fastapi.responses import JSONResponse, Response

from . import chat_brain
from .config import settings
from .tts_engine import VALID_STYLES, engine

router = APIRouter(prefix="/api")

# Lịch sử hội thoại ngắn cho robot (1 robot, trong bộ nhớ). Giữ vài lượt gần nhất.
_chat_history: deque = deque(maxlen=8)


def _check_text(text: str) -> str:
    text = (text or "").strip()
    if not text:
        raise HTTPException(status_code=400, detail="Thiếu văn bản cần đọc.")
    if len(text) > settings.max_chars:
        raise HTTPException(
            status_code=400,
            detail=f"Văn bản quá dài (tối đa {settings.max_chars} ký tự).",
        )
    return text


@router.get("/health")
def health() -> JSONResponse:
    return JSONResponse(
        {
            "status": "ok",
            "precision": settings.precision,
            "cloning_enabled": settings.enable_cloning,
            "max_chars": settings.max_chars,
            "styles": sorted(VALID_STYLES),
        }
    )


@router.get("/voices")
def voices() -> JSONResponse:
    try:
        return JSONResponse({"voices": engine.list_voices()})
    except Exception as exc:  # nạp model lỗi -> báo rõ cho UI
        raise HTTPException(status_code=500, detail=f"Không nạp được model: {exc}")


@router.get("/say")
def say(text: str, voice: str = "", style: str = "") -> Response:
    """Endpoint GET đơn giản cho thiết bị nhúng (ESP32 `connecttohost`).

    Gọi: /api/say?text=Xin%20ch%C3%A0o  -> trả thẳng WAV.
    `voice` để trống -> dùng giọng mặc định của model.
    """
    text = _check_text(text)
    try:
        chosen = voice.strip() or engine.default_voice()
        wav = engine.synthesize(text=text, voice=chosen, style=(style.strip() or None))
    except Exception as exc:
        raise HTTPException(status_code=500, detail=f"Lỗi tổng hợp giọng: {exc}")
    return Response(content=wav, media_type="audio/wav")


def _run_chat(user_text: str) -> Response:
    """LLM nghĩ câu trả lời -> VieNeu-TTS đọc -> trả WAV. Kèm text ở header (URL-encode)."""
    user_text = (user_text or "").strip()
    if not user_text:
        raise HTTPException(status_code=400, detail="Không nghe/nhận được nội dung.")
    try:
        reply = chat_brain.chat(user_text, list(_chat_history))
    except Exception as exc:
        raise HTTPException(status_code=502, detail=f"Lỗi LLM: {exc}")
    _chat_history.append({"role": "user", "content": user_text})
    _chat_history.append({"role": "assistant", "content": reply})
    print(f"[CHAT] Ban: {user_text}\n[CHAT] EMO: {reply}", flush=True)
    try:
        wav = engine.synthesize(reply, engine.default_voice(), None)
    except Exception as exc:
        raise HTTPException(status_code=500, detail=f"Lỗi tổng hợp giọng: {exc}")
    return Response(
        content=wav,
        media_type="audio/wav",
        headers={"X-User-Text": quote(user_text), "X-Reply-Text": quote(reply)},
    )


@router.get("/chat")
def chat_text(text: str) -> Response:
    """Chat bằng CHỮ (để test): /api/chat?text=... -> WAV câu trả lời."""
    return _run_chat(text)


@router.post("/chat-audio")
async def chat_audio(audio: UploadFile = File(...)) -> Response:
    """Chat bằng GIỌNG (robot gửi audio mic): STT -> LLM -> TTS -> WAV."""
    raw = await audio.read()
    try:
        user_text = chat_brain.transcribe(raw, audio.filename or "speech.wav")
    except Exception as exc:
        raise HTTPException(status_code=502, detail=f"Lỗi nhận dạng giọng: {exc}")
    return _run_chat(user_text)


@router.post("/chat-reset")
def chat_reset() -> JSONResponse:
    _chat_history.clear()
    return JSONResponse({"status": "cleared"})


@router.post("/tts")
def tts(
    text: str = Form(...),
    voice: str = Form(...),
    style: str = Form(""),
) -> Response:
    text = _check_text(text)
    if not voice.strip():
        raise HTTPException(status_code=400, detail="Chưa chọn giọng.")
    style = style.strip() or None
    try:
        wav = engine.synthesize(text=text, voice=voice, style=style)
    except Exception as exc:
        raise HTTPException(status_code=500, detail=f"Lỗi tổng hợp giọng: {exc}")
    return Response(content=wav, media_type="audio/wav")


@router.post("/clone")
async def clone(
    text: str = Form(...),
    ref_audio: UploadFile = File(...),
    denoise: bool = Form(True),
) -> Response:
    if not settings.enable_cloning:
        raise HTTPException(
            status_code=403,
            detail="Nhân bản giọng đang tắt trên máy chủ này (cần PyTorch).",
        )
    text = _check_text(text)

    suffix = os.path.splitext(ref_audio.filename or "ref.wav")[1] or ".wav"
    tmp_path = None
    try:
        with tempfile.NamedTemporaryFile(
            suffix=suffix, delete=False, dir=settings.data_dir
        ) as tmp:
            tmp_path = tmp.name
            tmp.write(await ref_audio.read())
        wav = engine.clone(text=text, ref_audio_path=tmp_path, denoise=denoise)
    except HTTPException:
        raise
    except Exception as exc:
        raise HTTPException(status_code=500, detail=f"Lỗi nhân bản giọng: {exc}")
    finally:
        if tmp_path and os.path.exists(tmp_path):
            try:
                os.remove(tmp_path)
            except OSError:
                pass
    return Response(content=wav, media_type="audio/wav")
