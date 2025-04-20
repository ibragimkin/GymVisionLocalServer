import cv2
import torch
import numpy as np
import mediapipe as mp
from mediapipe.python.solutions.drawing_utils import DrawingSpec
from mediapipe.python.solutions.pose import POSE_CONNECTIONS
from gym_exercise_processor.rep_tracker.rep_tracker import RepTracker
from gym_exercise_processor.exercise_classifier.default import DefaultClassifier

class DefaultProcessor:
    def __init__(self, model_path, seq_len=30):
        self._initialize_model(model_path)
        self._initialize_rep_tracker()
        self._initialize_visualization_styles()
        self.seq_len = seq_len
        self.CUTOFF = 10
        self.BODY_CONNECTIONS = self._filter_body_connections()

    def _initialize_model(self, model_path):
        self.labels_map = DefaultClassifier.get_id2l_map()
        self.device = DefaultClassifier.get_device()
        self.seq = []
        self.model = self._load_model(model_path)
    
    def _load_model(self, model_path):
        model = DefaultClassifier.get_model()
        state = torch.load(model_path, map_location=self.device, weights_only=False)
        model.load_state_dict(state)
        model.to(self.device)
        model.eval()
        return model

    def _initialize_rep_tracker(self):
        self.rep_tracker = RepTracker()
        self.pose = self.rep_tracker.pose

    def _initialize_visualization_styles(self):
        self.LANDMARK_STYLE = DrawingSpec(color=(0, 255, 0), thickness=3, circle_radius=5)
        self.CONNECTION_STYLE = DrawingSpec(color=(255, 0, 0), thickness=2)

    def _filter_body_connections(self):
        return frozenset(
            conn for conn in POSE_CONNECTIONS if conn[0] > self.CUTOFF and conn[1] > self.CUTOFF
        )
    
    def _preprocess_landmarks(self, landmarks):
        for idx, lm in enumerate(landmarks):
            if idx <= self.CUTOFF:
                lm.visibility = 0
        coords = np.array([[lm.x, lm.y] for lm in landmarks], dtype=np.float32)
        hip_center = self._calculate_hip_center(coords)
        coords -= hip_center
        return coords

    @staticmethod
    def _calculate_hip_center(coords):
        left_hip = mp.solutions.pose.PoseLandmark.LEFT_HIP.value
        right_hip = mp.solutions.pose.PoseLandmark.RIGHT_HIP.value
        return (coords[left_hip] + coords[right_hip]) * 0.5

    def _classify_exercise(self, seq):
        frames, num_joints, coords = np.array(seq).shape
        flat = np.array(seq).reshape(frames, num_joints * coords)
        x = torch.from_numpy(flat.T.astype(np.float32)).unsqueeze(0).to(self.device)
        with torch.no_grad():
            logits = self.model(x).cpu()
        return self._get_top_exercise_label(logits) if logits is not None else None

    @staticmethod
    def _flatten_sequence(seq):
        frames, num_joints, coords = np.array(seq).shape
        return np.array(seq).reshape(frames, num_joints * coords)

    def _get_top_exercise_label(self, logits):
        avg_probs = torch.softmax(logits, dim=1).mean(dim=0)
        top_label_idx = avg_probs.topk(1).indices.item()
        return self.labels_map[top_label_idx]

    def _update_rep_tracker(self, detected_exercise_type, landmarks):
        if detected_exercise_type != self.rep_tracker.exercise_type:
            self.rep_tracker.set_exercise_type(detected_exercise_type)
        self.rep_tracker.exercise.count_reps(landmarks)
        self.rep_tracker.exercise.evaluate_correctness(landmarks)

    def _prepare_visualization_data(self, detected_exercise_type, landmarks):
        return {
            "exercise_type": detected_exercise_type,
            "reps": self.rep_tracker.exercise.counter,
            "correct_reps": self.rep_tracker.exercise.correct_reps,
            "stage": self.rep_tracker.exercise.stage,
            "landmarks": [{"x": lm.x, "y": lm.y, "visibility": lm.visibility} for lm in landmarks],
            "connections": [{"start": conn[0], "end": conn[1]} for conn in self.BODY_CONNECTIONS],
        }

    def process_frame_to_json(self, frame):
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        res = self.pose.process(rgb)

        if res.pose_landmarks:
            coords = self._preprocess_landmarks(res.pose_landmarks.landmark)
            self._update_sequence(coords)
            if len(self.seq) == self.seq_len:
                detected_exercise_type = self._classify_exercise(self.seq)
                self._update_rep_tracker(detected_exercise_type, res.pose_landmarks.landmark)
                return self._prepare_visualization_data(detected_exercise_type, res.pose_landmarks.landmark)
        else:
            self.seq.clear()

        return self._empty_visualization_data()

    def _update_sequence(self, coords):
        self.seq.append(coords)
        if len(self.seq) > self.seq_len:
            self.seq.pop(0)

    @staticmethod
    def _empty_visualization_data():
        return {
            "exercise_type": None,
            "reps": 0,
            "correct_reps": 0,
            "stage": None,
            "landmarks": [],
            "connections": [],
        }

    def release(self):
        self.pose.close()