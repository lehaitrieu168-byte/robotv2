"""Bộ não trò chuyện: Groq Whisper (nghe -> chữ) + Groq LLM (nghĩ câu trả lời).

Dùng cho robot EMO: audio mic -> transcribe -> chat -> (VieNeu-TTS đọc ở routes).
Khoá API đọc từ biến môi trường GROQ_API_KEY (file .env, KHÔNG commit).
"""
from __future__ import annotations

import httpx

from .config import settings

GROQ_BASE = "https://api.groq.com/openai/v1"


def _headers() -> dict:
    if not settings.groq_api_key:
        raise RuntimeError("Chưa cấu hình GROQ_API_KEY trong .env")
    return {"Authorization": f"Bearer {settings.groq_api_key}"}


def transcribe(audio_bytes: bytes, filename: str = "speech.wav") -> str:
    """Nhận dạng giọng nói tiếng Việt (Groq Whisper) -> văn bản."""
    files = {"file": (filename, audio_bytes, "audio/wav")}
    data = {
        "model": settings.groq_stt_model,
        "language": "vi",
        "response_format": "json",
    }
    with httpx.Client(timeout=60) as client:
        resp = client.post(
            f"{GROQ_BASE}/audio/transcriptions",
            headers=_headers(),
            files=files,
            data=data,
        )
        resp.raise_for_status()
        return (resp.json().get("text") or "").strip()


def chat(user_text: str, history: list[dict]) -> str:
    """Sinh câu trả lời (Groq LLM) từ câu người dùng + lịch sử hội thoại."""
    messages = [{"role": "system", "content": settings.chat_system_prompt}]
    messages += history
    messages.append({"role": "user", "content": user_text})
    payload = {
        "model": settings.groq_llm_model,
        "messages": messages,
        "temperature": 0.7,
        "max_tokens": 200,
    }
    with httpx.Client(timeout=60) as client:
        resp = client.post(
            f"{GROQ_BASE}/chat/completions",
            headers={**_headers(), "Content-Type": "application/json"},
            json=payload,
        )
        resp.raise_for_status()
        return resp.json()["choices"][0]["message"]["content"].strip()
