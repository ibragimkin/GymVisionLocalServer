import os
import numpy as np
import torch
from torch.utils.data import Dataset
from torch.utils.data import DataLoader, random_split

class CachedPoseDataset(Dataset):
    def __init__(self, pose_root, labels_map):
        self.files = []
        self.map = labels_map
        for dp, _, fns in os.walk(pose_root):
            for fn in fns:
                if fn.endswith(".npy"):
                    label = os.path.basename(dp)
                    if label not in labels_map:
                        continue
                    self.files.append((os.path.join(dp, fn), label))

    def __len__(self): return len(self.files)
    
    def __getitem__(self, i):
        path, label = self.files[i]
        seq = np.load(path)               # (T, J, 2)
        x = torch.from_numpy(seq.reshape(seq.shape[0], -1)).T
        y = self.map[label]
        return x, y

def get_dataloaders(
    data_dir,
    labels_map,
    Dataset,
    batch_size,
    val_split,
    seed,
    num_workers
) -> tuple:
    full_ds = Dataset(data_dir, labels_map)
    total = len(full_ds)
    val_n = int(total * val_split)
    train_n = total - val_n
    train_ds, val_ds = random_split(
        full_ds, [train_n, val_n],
        generator=torch.Generator().manual_seed(seed)
    )
    train_loader = DataLoader(
        train_ds, batch_size=batch_size, shuffle=True,
        num_workers=num_workers, pin_memory=True
    )
    val_loader = DataLoader(
        val_ds, batch_size=batch_size, shuffle=False,
        num_workers=num_workers, pin_memory=True
    )
    return train_loader, val_loader