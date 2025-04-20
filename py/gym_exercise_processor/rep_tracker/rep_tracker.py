import cv2
from gym_exercise_processor.rep_tracker.exercises import BarbellBicepsCurl, PushUp, Squat, ShoulderPress, PullUp, Deadlift
from gym_exercise_processor.rep_tracker.exercises import mp_pose

class RepTracker:
    def __init__(self, exercise_type=None):
        self.pose = mp_pose.Pose(static_image_mode=False, model_complexity=1, smooth_landmarks=True)
        self.exercise_type = exercise_type
        if exercise_type:
            self.exercise = self._get_exercise_class(exercise_type)

    def set_exercise_type(self, exercise_type):
        self.exercise = self._get_exercise_class(exercise_type)
        if not self.exercise:
            raise ValueError(f"Unsupported exercise type: {exercise_type}")

    def _get_exercise_class(self, exercise_type):
        exercise_classes = {
            "barbell biceps curl": BarbellBicepsCurl,
            "push-up": PushUp,
            "squat": Squat,
            "shoulder press": ShoulderPress,
            "pull up": PullUp,
            "deadlift": Deadlift
        }
        exercise_class = exercise_classes.get(exercise_type.lower())
        if not exercise_class:
            raise ValueError(f"Unsupported exercise type: {exercise_type}")
        self.exercise_type = exercise_type
        return exercise_class()

    def process_frame(self, frame):
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        results = self.pose.process(rgb_frame)

        if results.pose_landmarks:
            self.exercise.count_reps(results.pose_landmarks.landmark)

    def get_count(self):
        return self.exercise.counter

    def get_correct_count(self):
        return self.exercise.correct_reps
