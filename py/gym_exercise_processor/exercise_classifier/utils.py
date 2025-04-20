import cv2
import numpy as np
import mediapipe as mp
import os
import shutil

mp_pose = mp.solutions.pose
POSE_LANDMARKS = mp_pose.PoseLandmark
NUM_LANDMARKS = len(mp_pose.PoseLandmark) 
_pose_detector = mp_pose.Pose(static_image_mode=False,
                             model_complexity=1,
                             smooth_landmarks=True)

def extract_pose(video_path, max_frames=128):
    cap = cv2.VideoCapture(video_path)
    seq = []
    while len(seq) < max_frames:
        ret, frame = cap.read()
        if not ret: break
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        res = _pose_detector.process(rgb)
        if res.pose_landmarks:
            coords = np.array([[lm.x, lm.y] for lm in res.pose_landmarks.landmark],
                              dtype=np.float32)
            hip = (coords[mp_pose.PoseLandmark.LEFT_HIP.value] +
                   coords[mp_pose.PoseLandmark.RIGHT_HIP.value]) * 0.5
            seq.append(coords - hip)
    cap.release()
    _pose_detector.reset()
    if len(seq) == 0:
        return np.zeros((max_frames, NUM_LANDMARKS, 2), dtype=np.float32)
    if len(seq) < max_frames:
        pad_shape = seq[0].shape  # (NUM_LANDMARKS, 2)
        padding = [np.zeros(pad_shape, dtype=np.float32)
                   for _ in range(max_frames - len(seq))]
        seq.extend(padding)
    else:
        seq = seq[:max_frames]

    return np.stack(seq, axis=0)  # shape: (max_frames, NUM_LANDMARKS, 2)


def extract_pose_segment(video_path: str,
                         start_frame: int,
                         num_frames: int,
                         max_frames: int = 128) -> np.ndarray:
    cap = cv2.VideoCapture(video_path)
    cap.set(cv2.CAP_PROP_POS_FRAMES, start_frame)
    frames = []
    for _ in range(num_frames):
        ret, frame = cap.read()
        if not ret:
            break
        frame = cv2.fastNlMeansDenoisingColored(frame, None, 3, 3, 7, 21)
        frames.append(frame)
    cap.release()

    pose = mp_pose.Pose(static_image_mode=False,
                        model_complexity=1,
                        smooth_landmarks=True)
    seq = []
    for frame in frames:
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        res = pose.process(rgb)
        if res.pose_landmarks:
            lm = res.pose_landmarks.landmark
            coords = np.array([[p.x, p.y] for p in lm], dtype=np.float32)
            hip_center = (
                coords[POSE_LANDMARKS.LEFT_HIP.value] +
                coords[POSE_LANDMARKS.RIGHT_HIP.value]
            ) * 0.5
            coords -= hip_center
            seq.append(coords)
    pose.close()

    if not seq:
        return np.zeros((max_frames, len(POSE_LANDMARKS), 2), dtype=np.float32)
    if len(seq) < max_frames:
        pad = [np.zeros_like(seq[0]) for _ in range(max_frames - len(seq))]
        seq.extend(pad)
    return np.stack(seq[:max_frames], axis=0)

def segment_generator(video_path: str,
                      max_duration: float,
                      seq_len: int):
    cap = cv2.VideoCapture(video_path)
    fps = cap.get(cv2.CAP_PROP_FPS) or 30.0
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    cap.release()

    segment_size = int(max_duration * fps)
    for start in range(0, total_frames, segment_size):
        yield extract_pose_segment(video_path, start, segment_size, max_frames=seq_len)

def copy_dir_contents(src: str, dst: str):
    if not os.path.isdir(src):
        raise FileNotFoundError(f"Expected source directory not found: {src}")
    os.makedirs(dst, exist_ok=True)
    for entry in os.listdir(src):
        src_path = os.path.join(src, entry)
        dst_path = os.path.join(dst, entry)
        if os.path.isdir(src_path):
            shutil.copytree(src_path, dst_path, dirs_exist_ok=True)
        else:
            shutil.copy2(src_path, dst_path)