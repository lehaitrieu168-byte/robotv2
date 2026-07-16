"""Điểm khởi tạo FastAPI: mount API + phục vụ giao diện web tĩnh."""
from __future__ import annotations

import os
from contextlib import asynccontextmanager

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles

from .config import settings
from .routes import router
from .tts_engine import engine


@asynccontextmanager
async def lifespan(app: FastAPI):
    # Nạp sẵn model nếu PRELOAD_MODEL=true (giúp lần gọi đầu từ robot không bị chờ lâu)
    if settings.preload_model:
        engine.warmup()
    yield


app = FastAPI(title="VieNeu-TTS Web", version="1.0.0", lifespan=lifespan)

# Cho phép truy cập chéo nguồn (vd trang web nhỏ trên ESP32 gọi /api/voices)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)
app.include_router(router)

# Phục vụ frontend tĩnh tại "/" (index.html, style.css, app.js)
_STATIC_DIR = os.path.join(os.path.dirname(__file__), "static")
app.mount("/", StaticFiles(directory=_STATIC_DIR, html=True), name="static")


def main() -> None:
    """Chạy trực tiếp: python -m app.main"""
    import uvicorn

    uvicorn.run(
        "app.main:app",
        host=settings.host,
        port=settings.port,
        reload=False,
    )


if __name__ == "__main__":
    main()
