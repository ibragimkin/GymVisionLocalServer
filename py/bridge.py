# ptz_server.py
from fastapi import FastAPI, HTTPException, Form
from onvif import ONVIFCamera
from threading import Lock
import cv_handler
app = FastAPI()

# глобальный кеш: camera_id → (ONVIFCamera, ptz, profile_token)
_camera_cache = {}
_cache_lock = Lock()

def get_or_create_client(cam_id, host, port, user, pwd):
    with _cache_lock:
        if cam_id in _camera_cache:
            return _camera_cache[cam_id]
        cam = ONVIFCamera(host, port, user, pwd)
        ptz = cam.create_ptz_service()
        media = cam.create_media_service()
        profile = media.GetProfiles()[0]  # первый профиль
        ctx = (cam, ptz, profile.token)
        _camera_cache[cam_id] = ctx
        return ctx

@app.post("/register")
def register(
        cam_id: str = Form(...),
        host:   str = Form(...),
        port:   int = Form(...),
        user:   str = Form(...),
        pwd:    str = Form(""),
    ):
    """Зарегистрировать (или обновить) камеру в кеше."""
    get_or_create_client(cam_id, host, port, user, pwd)
    return {"status": "registered", "camera_id": cam_id}

@app.post("/move")
def move(cam_id: str, x: float, y: float):
    """Начать непрерывное движение."""
    try:
        _, ptz, token = _camera_cache[cam_id]
    except KeyError:
        raise HTTPException(404, "Camera not registered")
    ptz.ContinuousMove({
        "ProfileToken": token,
        "Velocity": {"PanTilt": {"x": x, "y": y}}
    })
    return {"status": "moving", "camera_id": cam_id}

@app.post("/stop")
def stop(cam_id: str):
    """Остановить движение."""
    try:
        _, ptz, token = _camera_cache[cam_id]
    except KeyError:
        raise HTTPException(404, "Camera not registered")
    ptz.Stop({
        'ProfileToken': token,
        'PanTilt': True,
        'Zoom': True
    })
    return {"status": "stopped", "camera_id": cam_id}

@app.post("/start_stream_cv")
def start_stream_cv(rtsp_url: str, srt_url: str):
    cv_handler.start_stream(rtsp_url, srt_url)

    return {"status": "done"}