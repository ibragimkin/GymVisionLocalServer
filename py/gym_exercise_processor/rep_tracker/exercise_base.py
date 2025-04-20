import numpy as np
import mediapipe as mp

mp_pose = mp.solutions.pose

class ExerciseBase:
    def __init__(self):
        self.counter = 0        # Tracks the number of repetitions
        self.stage = None       # Tracks the stage of the exercise (e.g., "up", "down")
        self.correct_reps = 0   # Tracks the number of correct repetitions

    def count_reps(self, landmarks):
        raise NotImplementedError("Subclasses must implement this method.")

    def evaluate_correctness(self, landmarks):
        raise NotImplementedError("Subclasses must implement this method.")

    @staticmethod
    def calculate_angle(a, b, c):
        a = np.array([a.x, a.y])
        b = np.array([b.x, b.y])
        c = np.array([c.x, c.y])

        ba = a - b
        bc = c - b

        cosine_angle = np.dot(ba, bc) / (np.linalg.norm(ba) * np.linalg.norm(bc))
        angle = np.arccos(np.clip(cosine_angle, -1.0, 1.0))
        return np.degrees(angle)