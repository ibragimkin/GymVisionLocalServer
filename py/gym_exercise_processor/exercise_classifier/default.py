import shutil
import os
import torch
import torch.nn as nn
import torch.optim as optim
from kagglehub import dataset_download

from gym_exercise_processor.exercise_classifier.preprocess import preprocess_test, preprocess_train
from gym_exercise_processor.exercise_classifier.dataset import CachedPoseDataset, get_dataloaders
from gym_exercise_processor.exercise_classifier.model import ExerciseClassifier
from gym_exercise_processor.exercise_classifier.train import train_model
from gym_exercise_processor.exercise_classifier.infer import evaluate_model, compute_metrics


class DefaultClassifier:
    def __init__(self):
        print("Initializing DefaultClassifier...")
        self._initialize_paths()
        self._initialize_hyperparameters()
        self.device = self.get_device()
        self.labels_map = self.get_l2id_map()
        self.model = self.get_model().to(self.device)
        self.criterion = nn.CrossEntropyLoss()
        self.optimizer = optim.Adam(self.model.parameters(), lr=1e-3)

    def _initialize_paths(self):
        self.train_dir = "data/train"
        self.test_dir = "data/test"
        self.train_poses_dir = "data/train_poses"
        self.test_poses_dir = "data/test_poses"
        self.download_data = True

    def _initialize_hyperparameters(self):
        self.labels = self.get_labels()
        self.batch_size = 16
        self.val_split = 0.2
        self.num_workers = 4
        self.epochs = 60
        self.seq_len = 128
        self.max_duration = 10
        self.save_every = 0

    @staticmethod
    def get_labels():
        return sorted([
            "barbell biceps curl", "push-up", "squat",
            "shoulder press", "pull Up", "deadlift"
        ])

    @staticmethod
    def get_l2id_map():
        labels = DefaultClassifier.get_labels()
        return {label: idx for idx, label in enumerate(labels)}

    @staticmethod
    def get_id2l_map():
        labels = DefaultClassifier.get_labels()
        return {idx: label for idx, label in enumerate(labels)}

    @staticmethod
    def get_device():
        return torch.device("cuda" if torch.cuda.is_available() else "cpu")

    @staticmethod
    def get_model():
        model = ExerciseClassifier(
            num_joints=33,
            in_dim=2,
            hidden_dim=64,
            lstm_layers=2,
            num_classes=6
        )
        return model

    def _download_data(self, force=False):
        print("Checking if data needs to be downloaded...")
        if not force and os.path.exists(self.train_dir) and os.listdir(self.train_dir):
            print(f"Train data already exists in {self.train_dir}. Skipping download.")
            return
        if not force and os.path.exists(self.test_dir) and os.listdir(self.test_dir):
            print(f"Test data already exists in {self.test_dir}. Skipping download.")
            return

        print("Downloading dataset from Kaggle...")
        download_path = dataset_download("philosopher0808/gym-workoutexercises-video")
        try:
            shutil.copytree(
                os.path.join(download_path, "verified_data", "verified_data"),
                self.train_dir,
                dirs_exist_ok=True
            )
            print(f"Train data copied to {self.train_dir}.")
            shutil.copytree(
                os.path.join(download_path, "test", "test"),
                self.test_dir,
                dirs_exist_ok=True
            )
            print(f"Test data copied to {self.test_dir}.")
        finally:
            if force:
                shutil.rmtree(download_path)
                print("Temporary download directory removed.")

    def preprocess_data(self):
        print("Starting data preprocessing...")
        if self.download_data:
            self._download_data()
        preprocess_train(self.train_dir, self.train_poses_dir, self.labels)
        preprocess_test(self.test_dir, self.test_poses_dir, self.labels)
        print("Data preprocessing completed.")

    def train(self):
        print("Starting training process...")
        train_loader, val_loader = get_dataloaders(
            self.train_poses_dir,
            self.labels_map,
            CachedPoseDataset,
            self.batch_size,
            self.val_split,
            seed=42,
            num_workers=self.num_workers
        )
        print("Training and validation data loaders created.")
        self.best_model = train_model(
            self.model,
            self.criterion,
            self.optimizer,
            self.device,
            train_loader,
            val_loader,
            self.epochs,
            self.save_every
        )
        print("Training completed.")

    def evaluate(self):
        print("Starting evaluation process...")
        y_true, y1, y2, y3, results = evaluate_model(
            self.best_model,
            self.model,
            self.device,
            self.test_dir,
            self.labels_map,
            self.seq_len,
            self.max_duration,
            self.test_poses_dir
        )
        metrics = compute_metrics(y_true, y1, y2, y3, results)
        print("Evaluation completed. Metrics:")
        print(f"Top-1 Accuracy: {metrics['top1']:.4f}")
        print(f"Top-2 Accuracy: {metrics['top2']:.4f}")

    def run_pipeline(self):
        print("Running the full pipeline...")
        self.preprocess_data()
        self.train()
        self.evaluate()
        print("Pipeline execution completed.")
