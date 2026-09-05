"""Train and export the Mjolnir motion classifier.

The raw dataset contains 20 recordings each of IDLE, LIFT, SHAKE and SWING.
Each recording is reduced to eight inexpensive statistical features so the
same feature extraction can run on an ESP8266.
"""

from pathlib import Path
import numpy as np
import pandas as pd
from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import StratifiedKFold, cross_val_score
from sklearn.pipeline import make_pipeline
from sklearn.preprocessing import StandardScaler

DATASET = Path(__file__).with_name("mjolnir_dataset.csv")
FEATURES = [
    "az_max",
    "gyromag_mean",
    "accmag_mean",
    "gz_std",
    "gyromag_max",
    "az_mean",
    "gz_range",
    "gz_min",
]


def extract_features(frame: pd.DataFrame) -> list[float]:
    acc = frame[["ax", "ay", "az"]].to_numpy()
    gyro = frame[["gx", "gy", "gz"]].to_numpy()
    accmag = np.linalg.norm(acc, axis=1)
    gyromag = np.linalg.norm(gyro, axis=1)
    gz = frame["gz"].to_numpy()
    return [
        frame["az"].max(),
        gyromag.mean(),
        accmag.mean(),
        gz.std(ddof=0),
        gyromag.max(),
        frame["az"].mean(),
        gz.max() - gz.min(),
        gz.min(),
    ]


def main() -> None:
    df = pd.read_csv(DATASET)
    rows = []
    for (label, sample), frame in df.groupby(["label", "sample"]):
        rows.append([label, sample, *extract_features(frame)])

    features = pd.DataFrame(rows, columns=["label", "sample", *FEATURES])
    features.to_csv(Path(__file__).with_name("mjolnir_features.csv"), index=False)

    X = features[FEATURES]
    y = features["label"]
    model = make_pipeline(StandardScaler(), LogisticRegression(max_iter=2000))
    cv = StratifiedKFold(n_splits=5, shuffle=True, random_state=42)
    scores = cross_val_score(model, X, y, cv=cv)
    print(f"5-fold CV accuracy: {scores.mean():.3f} +/- {scores.std():.3f}")

    model.fit(X, y)
    scaler = model.named_steps["standardscaler"]
    classifier = model.named_steps["logisticregression"]
    print("classes:", classifier.classes_)
    print("mean:", scaler.mean_)
    print("scale:", scaler.scale_)
    print("coef:\n", classifier.coef_)
    print("bias:", classifier.intercept_)


if __name__ == "__main__":
    main()
