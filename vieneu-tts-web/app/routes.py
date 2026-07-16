"""Các endpoint API cho web TTS.

- GET  /api/health        -> trạng thái + cấu hình
- GET  /api/voices        -> danh sách giọng dựng sẵn
- POST /api/tts           -> đọc text bằng giọng dựng sẵn (WAV)
- POST /api/clone         -> nhân bản giọng từ audio mẫu (WAV)
"""
from __future__ import annotations

import os
import tempfile

from fastapi import APIRouter, File, Form, HTTPException, UploadFile
from fastapi.responses import JSONResponse, Response

from .config import settings
from .tts_engine import VALID_STYLES, engine

router = APIRouter(prefix="/api")


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
