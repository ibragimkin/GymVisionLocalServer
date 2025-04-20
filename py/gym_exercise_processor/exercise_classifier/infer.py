from sklearn.metrics import accuracy_score, classification_report
import torch
import numpy as np
import os
from gym_exercise_processor.exercise_classifier.utils import segment_generator

def evaluate_model(
    model_path,
    model,
    device,
    test_dir,
    labels_map,
    seq_len,
    max_duration,
    preprocessed_dir
) -> tuple:

    state = torch.load(model_path, map_location=device, weights_only=False)
    model.load_state_dict(state)
    model.to(device)
    model.eval()

    y_true, y1, y2, y3, results = [], [], [], [], []

    for label in sorted(os.listdir(test_dir)):
        if label not in labels_map:
            continue

        label_dir = os.path.join(test_dir, label)
        for fname in sorted(os.listdir(label_dir)):
            if not fname.lower().endswith('.mp4'):
                continue
            video_path = os.path.join(label_dir, fname)
            if preprocessed_dir:
                npy_path = os.path.join(
                    preprocessed_dir, label, os.path.splitext(fname)[0] + '.npy'
                )
                if os.path.isfile(npy_path):
                    data = np.load(npy_path)
                else:
                    data = np.stack(
                        list(segment_generator(video_path, max_duration, seq_len)),
                        axis=0
                    )
            else:
                data = np.stack(
                    list(segment_generator(video_path, max_duration, seq_len)),
                    axis=0
                )
            segment_logits = []
            for seg in data:
                frames, num_joints, coords = seg.shape
                flat = seg.reshape(frames, num_joints * coords)
                x = torch.from_numpy(flat.T.astype(np.float32)) \
                         .unsqueeze(0) \
                         .to(device)

                with torch.no_grad():
                    logits = model(x).cpu()
                segment_logits.append(logits)

            if not segment_logits:
                continue
            all_logits = torch.cat(segment_logits, dim=0)
            avg_probs = torch.softmax(all_logits, dim=1).mean(dim=0)
            topk = avg_probs.topk(3).indices.tolist()
            y_true.append(labels_map[label])
            y1.append(topk[0])
            y2.append(tuple(topk[:2]))
            y3.append(tuple(topk))
            results.append((label, fname, *topk))

    return y_true, y1, y2, y3, results


def compute_metrics(y_true, y1, y2, y3):
    acc1 = accuracy_score(y_true, y1)
    acc2 = sum(1 for yt, p in zip(y_true, y2) if yt in p) / len(y_true)
    acc3 = sum(1 for yt, p in zip(y_true, y3) if yt in p) / len(y_true)
    report = classification_report(y_true, y1, output_dict=True)
    return {'top1': acc1, 'top2': acc2, 'top3': acc3, 'report': report}
