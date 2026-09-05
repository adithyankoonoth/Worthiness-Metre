#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>
#include "model_parameters.h"
#include "web_page.h"

Adafruit_MPU6050 mpu;
ESP8266WebServer server(80);

const int SDA_PIN = 4;
const int SCL_PIN = 5;
const int WINDOW_SAMPLES = 40;
const int SAMPLE_DELAY_MS = 50;

float azSamples[WINDOW_SAMPLES];
float accMagSamples[WINDOW_SAMPLES];
float gyroMagSamples[WINDOW_SAMPLES];
float gzSamples[WINDOW_SAMPLES];

int lastPrediction = 0;
float lastConfidence = 0.0f;
float lastScores[4] = {0, 0, 0, 0};
bool hasResult = false;

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
    accMagSamples[i] = sqrtf(accel.acceleration.x * accel.acceleration.x + accel.acceleration.y * accel.acceleration.y + accel.acceleration.z * accel.acceleration.z);
    gyroMagSamples[i] = sqrtf(gyro.gyro.x * gyro.gyro.x + gyro.gyro.y * gyro.gyro.y + gyro.gyro.z * gyro.gyro.z);
    gzSamples[i] = gyro.gyro.z;
    delay(SAMPLE_DELAY_MS);
  }
}

int classify(float *features, float *scoresOut) {
  for (int c = 0; c < 4; ++c) {
    float score = MJ_BIAS[c];
    for (int i = 0; i < 8; ++i) {
      const float normalized = (features[i] - MJ_FEATURE_MEAN[i]) / MJ_FEATURE_STD[i];
      score += MJ_COEF[c][i] * normalized;
    }
    scoresOut[c] = score;
  }
  int best = 0;
  for (int c = 1; c < 4; ++c) if (scoresOut[c] > scoresOut[best]) best = c;
  return best;
}

float confidenceFromScores(const float *scores) {
  float maxScore = scores[0];
  for (int c = 1; c < 4; ++c) if (scores[c] > maxScore) maxScore = scores[c];
  float sumExp = 0.0f;
  for (int c = 0; c < 4; ++c) sumExp += expf(scores[c] - maxScore);
  return 100.0f / sumExp;
}

void evaluateHammer() {
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

  lastPrediction = classify(features, lastScores);
  lastConfidence = confidenceFromScores(lastScores);
  hasResult = true;

  Serial.println("\n===== MJOLNIR WORTHINESS EVALUATION =====");
  Serial.print("Motion: "); Serial.println(MJ_CLASSES[lastPrediction]);
  Serial.print("Confidence: "); Serial.print(lastConfidence, 1); Serial.println("%");
  Serial.print("Scores: ");
  for (int c = 0; c < 4; ++c) {
    Serial.print(MJ_CLASSES[c]); Serial.print("="); Serial.print(lastScores[c], 3);
    if (c < 3) Serial.print(" | ");
  }
  Serial.println();
  Serial.println(lastPrediction == 3 ? "\nWORTHY" : "\nNOT WORTHY");
  Serial.println(lastPrediction == 3 ? "The hammer has accepted you." : "Thor has declined your application.");
  Serial.println("==========================================");
}

void handleStatus() {
  String json = "{\"evaluated\":" + String(hasResult ? "true" : "false");
  json += ",\"motion\":\"" + String(MJ_CLASSES[lastPrediction]) + "\"";
  json += ",\"confidence\":" + String(lastConfidence, 1);
  json += ",\"worthy\":" + String(lastPrediction == 3 ? "true" : "false");
  json += ",\"score\":" + String(lastPrediction == 3 ? 100 : 0);
  json += ",\"message\":\"" + String(lastPrediction == 3 ? "The hammer has accepted you." : "Thor has declined your application.") + "\"}";
  server.send(200, "application/json", json);
}

void setupWebServer() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("MJOLNIR", "worthy123");

  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", MJOLNIR_WEB_PAGE);
  });
  server.on("/api/status", HTTP_GET, handleStatus);
  server.begin();

  Serial.println("\nMJOLNIR WiFi network: MJOLNIR");
  Serial.println("Password: worthy123");
  Serial.print("Dashboard: http://");
  Serial.println(WiFi.softAPIP());
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

  setupWebServer();
  Serial.println("Mjolnir TinyML evaluator ready.");
  Serial.println("Press any key in Serial Monitor to evaluate.");
}

void loop() {
  server.handleClient();

  if (Serial.available()) {
    while (Serial.available()) Serial.read();
    evaluateHammer();
  }
}
