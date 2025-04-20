import argparse
import cv2
import time
from gym_exercise_processor.real_time_processor.default import DefaultProcessor


def initialize_video_capture():
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        raise IOError("Error: Unable to access the webcam.")
    return cap


def get_video_properties(cap):
    frame_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    frame_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    fps = cap.get(cv2.CAP_PROP_FPS)
    return frame_width, frame_height, fps

def initialize_video_writer(output_video_path, frame_width, frame_height, fps):
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    return cv2.VideoWriter(output_video_path, fourcc, fps, (frame_width, frame_height))


def process_frame(processor, frame):
    data = processor.process_frame_to_json(frame)

    # Visualizing JSON
    if data["exercise_type"]:
        cv2.putText(frame, f"Exercise: {data['exercise_type']}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        cv2.putText(frame, f"Reps: {data['reps']}", (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        cv2.putText(frame, f"Correct Reps: {data['correct_reps']}", (10, 90), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        if data["stage"]:
            cv2.putText(frame, f"Stage: {data['stage']}", (10, 120), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
    else:
        cv2.putText(frame, "Exercise: None", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)

    landmarks = data["landmarks"]
    for landmark in landmarks:
        if landmark["visibility"] > 0.5:
            x = int(landmark["x"] * frame.shape[1])
            y = int(landmark["y"] * frame.shape[0])
            cv2.circle(frame, (x, y), 5, (0, 0, 255), -1)

    connections = data["connections"]
    for conn in connections:
        start = conn["start"]
        end = conn["end"]
        if start < len(landmarks) and end < len(landmarks):
            x1 = int(landmarks[start]["x"] * frame.shape[1])
            y1 = int(landmarks[start]["y"] * frame.shape[0])
            x2 = int(landmarks[end]["x"] * frame.shape[1])
            y2 = int(landmarks[end]["y"] * frame.shape[0])
            cv2.line(frame, (x1, y1), (x2, y2), (255, 0, 0), 2)

    return frame


def process_video(output_video_path, model_path, seq_len, fps):
    processor = DefaultProcessor(model_path, seq_len)
    cap = initialize_video_capture()
    frame_width, frame_height, _ = get_video_properties(cap)
    out = initialize_video_writer(output_video_path, frame_width, frame_height, fps)

    try:
        frame_interval = 1.0 / fps
        prev_time = time.time()
        while True:
            ret, frame = cap.read()
            if not ret:
                break

            processed_frame = process_frame(processor, frame)
            cv2.imshow('Exercise Classifier', processed_frame)
            out.write(processed_frame)

            now = time.time()
            dt = now - prev_time
            sleep_time = frame_interval - dt
            if sleep_time > 0:
                time.sleep(sleep_time)
            prev_time = now

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    finally:
        cap.release()
        out.release()
        processor.release()
        cv2.destroyAllWindows()

    print(f"Processed video saved to {output_video_path}")


def main():
    parser = argparse.ArgumentParser(description="Process webcam feed for gym exercise analysis.")
    parser.add_argument("--checkpoint", type=str, default="checkpoint.pth", help="Path to the model checkpoint file (default: model_checkpoint.pth).")
    parser.add_argument("--output", type=str, default="output.mp4", help="Path to save the processed output video (default: output_video.mp4).")
    parser.add_argument("--seq_len", type=int, default=128, help="Sequence length for the model (default: 128).")
    parser.add_argument("--fps", type=float, default=30, help="Override camera FPS for output video (default: 30).")

    args = parser.parse_args()

    model_path = args.checkpoint
    output_video_path = args.output
    seq_len = args.seq_len
    fps = args.fps

    process_video(output_video_path, model_path, seq_len, fps)


if __name__ == "__main__":
    main()