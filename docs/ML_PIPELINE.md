# Mjolnir TinyML Pipeline

## Overview

Mjolnir classifies four motion classes from an MPU6050 mounted inside the hammer:

- `IDLE`
- `LIFT`
- `SHAKE`
- `SWING`

The embedded model is a multiclass logistic-regression classifier. It was chosen because the inference is small enough for the ESP8266: feature extraction is statistical and the classifier only needs four linear score calculations.

## Dataset

The recorded dataset contains 80 motion windows: 20 recordings for each class. Each raw recording contains accelerometer (`ax`, `ay`, `az`) and gyroscope (`gx`, `gy`, `gz`) readings.

The training script groups by both `label` and `sample`. This is important because sample numbers 1–20 repeat for every class.

## Feature extraction

Each recording is reduced to eight features:

| # | Feature | Meaning |
|---|---|---|
| 0 | `az_max` | Maximum Z-axis acceleration |
| 1 | `gyromag_mean` | Mean gyroscope magnitude |
| 2 | `accmag_mean` | Mean acceleration magnitude |
| 3 | `gz_std` | Standard deviation of Z-axis gyro |
| 4 | `gyromag_max` | Maximum gyroscope magnitude |
| 5 | `az_mean` | Mean Z-axis acceleration |
| 6 | `gz_range` | Maximum Z gyro minus minimum Z gyro |
| 7 | `gz_min` | Minimum Z-axis gyro |

Acceleration magnitude is calculated as `sqrt(ax² + ay² + az²)` and gyro magnitude as `sqrt(gx² + gy² + gz²)`.

## Training

`ml/train_model.py` reproduces the feature extraction and trains a standardized multiclass logistic-regression model.

Place the raw `mjolnir_dataset.csv` beside the training script, then run:

```bash
python train_model.py
```

The script prints the 5-fold cross-validation score and the scaler/model parameters used by the ESP8266 firmware.

## Embedded inference

The ESP8266 collects a 40-sample window at 20 Hz, producing a roughly two-second observation window.

For every class, the firmware calculates:

```text
score = bias + Σ(coefficient × standardized_feature)
```

The class with the largest score wins. No probability/softmax calculation is required for the embedded decision.

The generated parameters are stored in `ml/model_parameters.h` and are included by `firmware/mjolnir_tinyml.ino`.

## Hardware

```text
MPU6050       ESP8266
VCC      →    3V3
GND      →    GND
SDA      →    D2 / GPIO4
SCL      →    D1 / GPIO5
AD0      →    GND
INT      →    not connected
```

The firmware uses the Adafruit MPU6050 library and `Wire.begin(4, 5)`.

## Current model note

The model is trained from the initial single-device dataset. Before treating the classifier as robust, collect additional windows from different people and different hammer handling styles, then retrain and compare validation performance.
