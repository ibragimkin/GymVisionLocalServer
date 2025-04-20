from pathlib import Path
import torch
from tqdm import tqdm

def train_model(
    model,
    criterion,
    optimizer,
    device,
    train_loader,
    val_loader,
    epochs,
    save_every
):
    Path("checkpoints").mkdir(parents=True, exist_ok=True)
    best_acc = 0.0
    best_path = ''

    for epoch in range(1, epochs + 1):
        model.train()
        train_loss, train_acc = 0.0, 0.0

        for x, y in tqdm(train_loader, desc=f"Epoch {epoch}/{epochs} [Train]", leave=False):
            x, y = x.to(device), y.to(device)
            optimizer.zero_grad()
            logits = model(x)
            loss = criterion(logits, y)
            loss.backward()
            optimizer.step()
            train_loss += loss.item() * x.size(0)
            train_acc += (logits.argmax(1) == y).sum().item()

        train_loss /= len(train_loader.dataset)
        train_acc /= len(train_loader.dataset)

        model.eval()
        val_loss, val_acc = 0.0, 0.0
        with torch.no_grad():
            for x, y in tqdm(val_loader, desc=f"Epoch {epoch}/{epochs} [Val]", leave=False):
                x, y = x.to(device), y.to(device)
                logits = model(x)
                loss = criterion(logits, y)
                val_loss += loss.item() * x.size(0)
                val_acc += (logits.argmax(1) == y).sum().item()

        val_loss /= len(val_loader.dataset)
        val_acc /= len(val_loader.dataset)

        print(f"[Epoch {epoch:02d}] "
              f"Train Acc: {train_acc:.4f} | Val Acc: {val_acc:.4f} | "
              f"Train Loss: {train_loss:.4f} | Val Loss: {val_loss:.4f}")

        if val_acc > best_acc:
            best_acc = val_acc
            best_path = "checkpoints/checkpoint_best.pth"
            torch.save(model.state_dict(), best_path)

        if save_every != 0 and epoch % save_every == 0:
            path = f"checkpoints/checkpoint_epoch{epoch}.pth"
            torch.save(model.state_dict(), path)

    print(f"Training complete. Best model saved to: {best_path}")
    return best_path
