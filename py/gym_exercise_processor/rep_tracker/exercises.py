from gym_exercise_processor.rep_tracker.exercise_base import ExerciseBase
from gym_exercise_processor.rep_tracker.exercise_base import mp_pose

class BarbellBicepsCurl(ExerciseBase):
    def count_reps(self, landmarks):
        left_elbow_angle = self.calculate_angle(
            landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value],
            landmarks[mp_pose.PoseLandmark.LEFT_ELBOW.value],
            landmarks[mp_pose.PoseLandmark.LEFT_WRIST.value]
        )

        if left_elbow_angle > 160:
            self.stage = "down"
        if left_elbow_angle < 50 and self.stage == "down":
            self.stage = "up"
            self.counter += 1
            score = self.evaluate_correctness(landmarks)

            if score >= 50:
                self.correct_reps += 1

    def evaluate_correctness(self, landmarks):
        left_elbow_angle = self.calculate_angle(
            landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value],
            landmarks[mp_pose.PoseLandmark.LEFT_ELBOW.value],
            landmarks[mp_pose.PoseLandmark.LEFT_WRIST.value]
        )

        ideal_elbow_angle_range = (50, 160)
        score = 100

        if not (ideal_elbow_angle_range[0] <= left_elbow_angle <= ideal_elbow_angle_range[1]):
            score -= 50

        return max(score, 0)


class PushUp(ExerciseBase):
    def count_reps(self, landmarks):
        shoulder_hip_distance = abs(
            landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value].y -
            landmarks[mp_pose.PoseLandmark.LEFT_HIP.value].y
        )

        if shoulder_hip_distance > 0.2:
            self.stage = "down"
        if shoulder_hip_distance < 0.1 and self.stage == "down":
            self.stage = "up"
            self.counter += 1
            score = self.evaluate_correctness(landmarks)

            if score >= 50:
                self.correct_reps += 1

    def evaluate_correctness(self, landmarks):
        shoulder_hip_distance = abs(
            landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value].y -
            landmarks[mp_pose.PoseLandmark.LEFT_HIP.value].y
        )
        left_elbow_angle = self.calculate_angle(
            landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value],
            landmarks[mp_pose.PoseLandmark.LEFT_ELBOW.value],
            landmarks[mp_pose.PoseLandmark.LEFT_WRIST.value]
        )

        max_shoulder_hip_distance = 0.2
        ideal_elbow_angle_range = (80, 100)
        score = 100

        if shoulder_hip_distance > max_shoulder_hip_distance:
            score -= 40

        if not (ideal_elbow_angle_range[0] <= left_elbow_angle <= ideal_elbow_angle_range[1]):
            score -= 50

        return max(score, 0)


class Squat(ExerciseBase):
    def count_reps(self, landmarks):
        left_knee_angle = self.calculate_angle(
            landmarks[mp_pose.PoseLandmark.LEFT_HIP.value],
            landmarks[mp_pose.PoseLandmark.LEFT_KNEE.value],
            landmarks[mp_pose.PoseLandmark.LEFT_ANKLE.value]
        )

        if left_knee_angle > 160:
            self.stage = "up"
        if left_knee_angle < 90 and self.stage == "up":
            self.stage = "down"
            self.counter += 1
            score = self.evaluate_correctness(landmarks)

            if score >= 50:
                self.correct_reps += 1


    def evaluate_correctness(self, landmarks):
        left_knee_angle = self.calculate_angle(
            landmarks[mp_pose.PoseLandmark.LEFT_HIP.value],
            landmarks[mp_pose.PoseLandmark.LEFT_KNEE.value],
            landmarks[mp_pose.PoseLandmark.LEFT_ANKLE.value]
        )
        hip_knee_alignment = abs(
            landmarks[mp_pose.PoseLandmark.LEFT_HIP.value].x -
            landmarks[mp_pose.PoseLandmark.LEFT_KNEE.value].x
        )

        ideal_knee_angle_range = (80, 100)
        max_alignment_deviation = 0.1
        score = 100

        if not (ideal_knee_angle_range[0] <= left_knee_angle <= ideal_knee_angle_range[1]):
            score -= 50

        if hip_knee_alignment > max_alignment_deviation:
            score -= 30

        return max(score, 0)


class ShoulderPress(ExerciseBase):
    def count_reps(self, landmarks):
        left_elbow_angle = self.calculate_angle(
            landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value],
            landmarks[mp_pose.PoseLandmark.LEFT_ELBOW.value],
            landmarks[mp_pose.PoseLandmark.LEFT_WRIST.value]
        )

        if left_elbow_angle > 160:
            self.stage = "up"
        if left_elbow_angle < 90 and self.stage == "up":
            self.stage = "down"
            self.counter += 1
            score = self.evaluate_correctness(landmarks)

            if score >= 50:
                self.correct_reps += 1


    def evaluate_correctness(self, landmarks):
        left_elbow_angle = self.calculate_angle(
            landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value],
            landmarks[mp_pose.PoseLandmark.LEFT_ELBOW.value],
            landmarks[mp_pose.PoseLandmark.LEFT_WRIST.value]
        )

        ideal_elbow_angle_range = (90, 160)
        score = 100

        if not (ideal_elbow_angle_range[0] <= left_elbow_angle <= ideal_elbow_angle_range[1]):
            score -= 50

        return max(score, 0)


class PullUp(ExerciseBase):
    def count_reps(self, landmarks):
        wrist_height = landmarks[mp_pose.PoseLandmark.LEFT_WRIST.value].y
        shoulder_height = landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value].y

        if wrist_height > shoulder_height + 0.1:
            self.stage = "down"
        if wrist_height < shoulder_height and self.stage == "down":
            self.stage = "up"
            self.counter += 1
            score = self.evaluate_correctness(landmarks)

            if score >= 50:
                self.correct_reps += 1


    def evaluate_correctness(self, landmarks):
        wrist_height = landmarks[mp_pose.PoseLandmark.LEFT_WRIST.value].y
        shoulder_height = landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value].y

        max_wrist_to_shoulder_distance = 0.1
        score = 100

        if wrist_height > shoulder_height + max_wrist_to_shoulder_distance:
            score -= 50

        return max(score, 0)


class Deadlift(ExerciseBase):
    def count_reps(self, landmarks):
        left_hip_angle = self.calculate_angle(
            landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value],
            landmarks[mp_pose.PoseLandmark.LEFT_HIP.value],
            landmarks[mp_pose.PoseLandmark.LEFT_KNEE.value]
        )

        if left_hip_angle > 160:
            self.stage = "up"
        if left_hip_angle < 90 and self.stage == "up":
            self.stage = "down"
            self.counter += 1
            score = self.evaluate_correctness(landmarks)

            if score >= 50:
                self.correct_reps += 1


    def evaluate_correctness(self, landmarks):
        left_hip_angle = self.calculate_angle(
            landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value],
            landmarks[mp_pose.PoseLandmark.LEFT_HIP.value],
            landmarks[mp_pose.PoseLandmark.LEFT_KNEE.value]
        )

        ideal_hip_angle_range = (90, 160)
        score = 100

        if not (ideal_hip_angle_range[0] <= left_hip_angle <= ideal_hip_angle_range[1]):
            score -= 50

        return max(score, 0)