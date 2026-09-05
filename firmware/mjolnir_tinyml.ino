#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>
#include "model_parameters.h"

Adafruit_MPU6050 mpu;

const int SDA_PIN = 4;  // ESP8266 D2
const int SCL_PIN = 5;  // ESP8266 D1
const int WINDOW_SAMPLES = 40;
const int SAMPLE_DELAY_MS = 50;

float azSamples[WINDOW_SAMPLES];
float accMagSamples[WINDOW_SAMPLES];
float gyroMagSamples[WINDOW_SAMPLES];
float gzSamples[WINDOW_SAMPLES];

float meanOf(const float *x, int n) {
  float sum = 0.0f;
  for (int i = 0; i < n; ++i) sum += x[i];
  return sum / n;
}

float minOf(const float *x, int n) {
  float v = x[0];
  for (int i = 1; i < n; ++i) if (x[i] < v) v = x[i];
  return v;
}

float maxOf(const float *x, int n) {
  float v = x[0];
  for (int i = 1; i < n; ++i) if (x[i] > v) v = x[i];
  return v;
}

float stdOf(const float *x, int n) {
  const float mean = meanOf(x, n);
  float sum = 0.0f;
  for (int i = 0; i < n; ++i) {
    const float d = x[i] - mean;
    sum += d * d;
  }
  return sqrtf(sum / n);
}

void collectWindow() {
  Serial.println("\nHold the hammer still, then perform a motion...");
  for (int i = 0; i < WINDOW_SAMPLES; ++i) {
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);

    azSamples[i] = accel.acceleration.z;
    accMagSamples[i] = sqrtf(
      accel.acceleration.x * accel.acceleration.x +
      accel.acceleration.y * accel.acceleration.y +
      accel.acceleration.z * accel.acceleration.z
    );
    gyroMagSamples[i] = sqrtf(
      gyro.gyro.x * gyro.gyro.x +
      gyro.gyro.y * gyro.gyro.y +
      gyro.gyro.z * gyro.gyro.z
    );
    gzSamples[i] = gyro.gyro.z;

    delay(SAMPLE_DELAY_MS);
  }
}

int classify(float *features, float *scoresOut) {
  for (int c = 0; c < 4; ++c) {
    float score = MJ_BIAS[c];
    for (int i = 0; i < 8; ++i) {
      const float normalized =
        (features[i] - MJ_FEATURE_MEAN[i]) / MJ_FEATURE_STD[i];
      score += MJ_COEF[c][i] * normalized;
    }
    scoresOut[c] = score;
  }

  int best = 0;
  for (int c = 1; c < 4; ++c) {
    if (scoresOut[c] > scoresOut[best]) best = c;
  }
  return best;
}

void printResult(int prediction, const float *features, const float *scores) {
  Serial.println("\n===== MJOLNIR WORTHINESS EVALUATION =====");
  Serial.print("Motion: ");
  Serial.println(MJ_CLASSES[prediction]);

  Serial.print("Scores: ");
  for (int c = 0; c < 4; ++c) {
    Serial.print(MJ_CLASSES[c]);
    Serial.print("=");
    Serial.print(scores[c], 3);
    if (c < 3) Serial.print(" | ");
  }
  Serial.println();

  if (prediction == 3) {
    Serial.println("WORTHY");
    Serial.println("The hammer has accepted you.");
  } else {
    Serial.println("NOT WORTHY");
    Serial.println("Thor has declined your application.");
  }
  Serial.println("==========================================");
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found. Check wiring.");
    while (true) delay(1000);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("Mjolnir TinyML evaluator ready.");
  Serial.println("Press any key in Serial Monitor to evaluate.");
}

void loop() {
  if (Serial.available()) {
    while (Serial.available()) Serial.read();

    collectWindow();

    float features[8];
    features[0] = maxOf(azSamples, WINDOW_SAMPLES);
    features[1] = meanOf(gyroMagSamples, WINDOW_SAMPLES);
    features[2] = meanOf(accMagSamples, WINDOW_SAMPLES);
    features[3] = stdOf(gzSamples, WINDOW_SAMPLES);
    features[4] = maxOf(gyroMagSamples, WINDOW_SAMPLES);
    features[5] = meanOf(azSamples, WINDOW_SAMPLES);
    features[6] = maxOf(gzSamples, WINDOW_SAMPLES) - minOf(gzSamples, WINDOW_SAMPLES);
    features[7] = minOf(gzSamples, WINDOW_SAMPLES);

    float scores[4];
    const int prediction = classify(features, scores);
    printResult(prediction, features, scores);
  }
}
