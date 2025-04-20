import cv2
import subprocess
from gym_exercise_processor.real_time_processor.default import DefaultProcessor

def start_stream(rtsp_url, srt_uri):
    # rtsp_url = "rtsp://anisa13125:anisanesnitch@192.168.0.37:554/stream1"
    cap = cv2.VideoCapture(rtsp_url)
    if not cap.isOpened():
        print("Не удалось открыть RTSP‐поток")
        return

    # fps = cap.get(cv2.CAP_PROP_FPS) or 12.0
    fps = 12.0
    w   = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    h   = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

    # задаём SRT-приёмник на локалхосте
    # srt_uri = "srt://0.0.0.0:7002?mode=listener&latency=0"

    ffmpeg_cmd = [
        "ffmpeg",
        "-y",
        "-f", "rawvideo",
        "-pixel_format", "bgr24",
        "-video_size", f"{w}x{h}",
        "-framerate", str(fps),
        "-i", "pipe:0",
        "-c:v", "libx264",
        "-preset", "veryfast",
        "-tune", "zerolatency",
        "-f", "mpegts",
        "-flush_packets", "0",
        srt_uri
    ]
    ffmpeg = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)

    processor = DefaultProcessor("../py/checkpoint.pth", seq_len=128)

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                break

            data = processor.process_frame_to_json(frame)

            # — ОТРисовываем поверх frame:
            if data["exercise_type"]:
                cv2.putText(frame, f"{data['exercise_type']} {data['reps']}/{data['correct_reps']}",
                            (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0,255,0), 2)
                if data["stage"]:
                    cv2.putText(frame, data["stage"], (10, 60),
                                cv2.FONT_HERSHEY_SIMPLEX, 1, (0,255,0), 2)
            else:
                cv2.putText(frame, "Exercise: None", (10,30),
                            cv2.FONT_HERSHEY_SIMPLEX, 1, (0,0,255), 2)

            for lm in data["landmarks"]:
                if lm["visibility"] > 0.5:
                    x, y = int(lm["x"]*w), int(lm["y"]*h)
                    cv2.circle(frame, (x,y), 5, (0,0,255), -1)
            for conn in data["connections"]:
                s,e = conn["start"], conn["end"]
                if s < len(data["landmarks"]) and e < len(data["landmarks"]):
                    x1 = int(data["landmarks"][s]["x"]*w)
                    y1 = int(data["landmarks"][s]["y"]*h)
                    x2 = int(data["landmarks"][e]["x"]*w)
                    y2 = int(data["landmarks"][e]["y"]*h)
                    cv2.line(frame,(x1,y1),(x2,y2),(255,0,0),2)

            # отправляем именно этот frame в ffmpeg:
            ffmpeg.stdin.write(frame.tobytes())
    finally:
        cap.release()
        ffmpeg.stdin.close()
        ffmpeg.wait()
        processor.release()
