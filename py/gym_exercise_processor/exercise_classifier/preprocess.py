from tqdm import tqdm
import os
import numpy as np
import cv2
from gym_exercise_processor.exercise_classifier.utils import extract_pose, extract_pose_segment

def preprocess_directory(input_dir, output_dir, is_train, labels_list, max_duration=None, frames=None):
    os.makedirs(output_dir, exist_ok=True)
    for dataset_or_label in tqdm(os.listdir(input_dir), desc="Processing", unit="folder"):
        src_dir = os.path.join(input_dir, dataset_or_label)
        if not os.path.isdir(src_dir):
            continue
        label_dirs = (
            os.listdir(src_dir) if is_train else [dataset_or_label]
        )
        for label in tqdm(label_dirs, desc=f"{dataset_or_label} → Labels", unit="label", leave=False):
            if label not in labels_list:
                continue
            in_dir = os.path.join(src_dir, label) if is_train else src_dir
            out_dir = os.path.join(output_dir, dataset_or_label if is_train else label, label if is_train else "")
            os.makedirs(out_dir, exist_ok=True)
            video_files = [f for f in os.listdir(in_dir) if f.endswith(".mp4")]
            for vid in tqdm(video_files, desc=f"{label} → Videos", unit="video", leave=False):
                video_path = os.path.join(in_dir, vid)
                save_path = os.path.join(out_dir, os.path.splitext(vid)[0] + ".npy")
                if os.path.exists(save_path):
                    continue
                if is_train:
                    seq = extract_pose(video_path)
                    np.save(save_path, seq)
                else:
                    cap = cv2.VideoCapture(video_path)
                    fps = cap.get(cv2.CAP_PROP_FPS) or 30.0
                    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
                    cap.release()
                    segment_size = int(max_duration * fps)
                    segments = []
                    for start in range(0, total_frames, segment_size):
                        seg = extract_pose_segment(video_path, start, segment_size, max_frames=frames)
                        segments.append(seg)
                    segments = np.stack(segments, axis=0)
                    np.save(save_path, segments)

def preprocess_train(train_dir, preprocessed_dir, labels_list):
    preprocess_directory(train_dir, preprocessed_dir, True, labels_list)

def preprocess_test(test_dir, preprocessed_dir, labels_list, max_duration=2.0, frames=60):
    preprocess_directory(test_dir, preprocessed_dir, False, labels_list, max_duration=max_duration, frames=frames)
