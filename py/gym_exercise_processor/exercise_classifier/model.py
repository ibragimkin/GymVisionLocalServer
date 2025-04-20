import torch.nn as nn

class STGCNBlock(nn.Module):
    def __init__(self, in_channels, out_channels, kernel_size):
        super().__init__()
        self.gcn = nn.Conv1d(in_channels, out_channels, kernel_size=1)
        self.tcn = nn.Sequential(
            nn.Conv1d(out_channels, out_channels, kernel_size, padding=kernel_size//2),
            nn.BatchNorm1d(out_channels),
            nn.ReLU(inplace=True)
        )
    def forward(self, x):
        # x: (B, C, T)
        x = self.gcn(x)
        x = self.tcn(x)
        return x

class ExerciseClassifier(nn.Module):
    def __init__(self, num_joints, in_dim, hidden_dim,
                 lstm_layers, num_classes):
        super().__init__()
        C = num_joints * in_dim
        self.stgcn1 = STGCNBlock(C, hidden_dim, kernel_size=9)
        self.stgcn2 = STGCNBlock(hidden_dim, hidden_dim, kernel_size=9)
        self.lstm = nn.LSTM(hidden_dim, hidden_dim, lstm_layers,
                            batch_first=True, bidirectional=True)
        self.fc = nn.Linear(hidden_dim*2, num_classes)

    def forward(self, x):
        # x: (B, C, T)
        x = self.stgcn1(x)
        x = self.stgcn2(x)
        # transpose to (B, T, hidden)
        x = x.permute(0, 2, 1)
        out, _ = self.lstm(x)
        # take last time step
        feat = out[:, -1, :]
        return self.fc(feat)
