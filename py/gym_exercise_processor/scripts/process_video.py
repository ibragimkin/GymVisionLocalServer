import argparse
import cv2
from gym_exercise_processor.real_time_processor.default import DefaultProcessor



def initialize_video_capture(input_video_path):
    cap = cv2.VideoCapture(input_video_path)
    if not cap.isOpened():
        raise IOError("Error: Unable to open input video.")
    return cap


def get_video_properties(cap):
    frame_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    frame_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    fps = int(cap.get(cv2.CAP_PROP_FPS))
    return frame_width, frame_height, fps


def initialize_video_writer(output_video_path, frame_width, frame_height, fps):
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    return cv2.VideoWriter(output_video_path, fourcc, fps, (frame_width, frame_height))


def process_frame(processor, frame):
    data = processor.process_frame_to_json(frame)

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


def process_video(input_video_path, output_video_path, model_path, seq_len):
    processor = DefaultProcessor(model_path, seq_len)
    cap = initialize_video_capture(input_video_path)
    frame_width, frame_height, fps = get_video_properties(cap)
    out = initialize_video_writer(output_video_path, frame_width, frame_height, fps)

    try:
        while cap.isOpened():
            ret, frame = cap.read()
            if not ret:
                break
            processed_frame = process_frame(processor, frame)
            out.write(processed_frame)
    finally:
        cap.release()
        out.release()
        processor.release()

    print(f"Processed video saved to {output_video_path}")

def main():
    parser = argparse.ArgumentParser(description="Process a video for gym exercise analysis.")
    parser.add_argument("--checkpoint", type=str, default="checkpoint.pth", help="Path to the model checkpoint file (default: model_checkpoint.pth).")
    parser.add_argument("--input", type=str, default="input.mp4", help="Path to the input video file (default: input_video.mp4).")
    parser.add_argument("--output", type=str, default="output.mp4", help="Path to save the processed output video (default: output_video.mp4).")
    parser.add_argument("--seq_len", type=int, default=128, help="Sequence length for the model (default: 128).")

    args = parser.parse_args()

    model_path = args.checkpoint
    input_video_path = args.input
    output_video_path = args.output
    seq_len = args.seq_len

    process_video(input_video_path, output_video_path, model_path, seq_len)

if __name__ == "__main__":
    main()