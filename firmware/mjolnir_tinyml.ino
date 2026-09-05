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
bool evaluationRequested = false;

float meanOf(const float *values, int count) {
  float sum = 0.0f;
  for (int i = 0; i < count; i++) sum += values[i];
  return sum / count;
}

float minOf(const float *values, int count) {
  float value = values[0];
  for (int i = 1; i < count; i++) {
    if (values[i] < value) value = values[i];
  }
  return value;
}

float maxOf(const float *values, int count) {
  float value = values[0];
  for (int i = 1; i < count; i++) {
    if (values[i] > value) value = values[i];
  }
  return value;
}

float stdOf(const float *values, int count) {
  float mean = meanOf(values, count);
  float sum = 0.0f;

  for (int i = 0; i < count; i++) {
    float difference = values[i] - mean;
    sum += difference * difference;
  }

  return sqrtf(sum / count);
}

void collectWindow() {
  Serial.println();
  Serial.println("===== MJOLNIR TRIAL STARTED =====");
  Serial.println("Hold the hammer ready...");
  Serial.println("Perform your motion NOW.");

  for (int i = 0; i < WINDOW_SAMPLES; i++) {
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temperature;

    mpu.getEvent(&accel, &gyro, &temperature);

    float ax = accel.acceleration.x;
    float ay = accel.acceleration.y;
    float az = accel.acceleration.z;

    float gx = gyro.gyro.x;
    float gy = gyro.gyro.y;
    float gz = gyro.gyro.z;

    azSamples[i] = az;
    accMagSamples[i] = sqrtf(ax * ax + ay * ay + az * az);
    gyroMagSamples[i] = sqrtf(gx * gx + gy * gy + gz * gz);
    gzSamples[i] = gz;

    delay(SAMPLE_DELAY_MS);
  }
}

int classify(float *features, float *scoresOut) {
  for (int c = 0; c < 4; c++) {
    float score = MJ_BIAS[c];

    for (int i = 0; i < 8; i++) {
      float normalized =
        (features[i] - MJ_FEATURE_MEAN[i]) / MJ_FEATURE_STD[i];

      score += MJ_COEF[c][i] * normalized;
    }

    scoresOut[c] = score;
  }

  int bestClass = 0;

  for (int c = 1; c < 4; c++) {
    if (scoresOut[c] > scoresOut[bestClass]) {
      bestClass = c;
    }
  }

  return bestClass;
}

float confidenceFromScores(const float *scores) {
  float maximum = scores[0];

  for (int c = 1; c < 4; c++) {
    if (scores[c] > maximum) maximum = scores[c];
  }

  float sumExp = 0.0f;

  for (int c = 0; c < 4; c++) {
    sumExp += expf(scores[c] - maximum);
  }

  return 100.0f / sumExp;
}

void evaluateHammer() {
  evaluationRequested = false;
  collectWindow();

  float features[8];

  features[0] = maxOf(azSamples, WINDOW_SAMPLES);
  features[1] = meanOf(gyroMagSamples, WINDOW_SAMPLES);
  features[2] = meanOf(accMagSamples, WINDOW_SAMPLES);
  features[3] = stdOf(gzSamples, WINDOW_SAMPLES);
  features[4] = maxOf(gyroMagSamples, WINDOW_SAMPLES);
  features[5] = meanOf(azSamples, WINDOW_SAMPLES);
  features[6] = maxOf(gzSamples, WINDOW_SAMPLES)
                 - minOf(gzSamples, WINDOW_SAMPLES);
  features[7] = minOf(gzSamples, WINDOW_SAMPLES);

  lastPrediction = classify(features, lastScores);
  lastConfidence = confidenceFromScores(lastScores);
  hasResult = true;

  Serial.println();
  Serial.println("===== MJOLNIR WORTHINESS EVALUATION =====");
  Serial.print("Motion: ");
  Serial.println(MJ_CLASSES[lastPrediction]);

  Serial.print("Confidence: ");
  Serial.print(lastConfidence, 1);
  Serial.println("%");

  Serial.print("Scores: ");
  for (int c = 0; c < 4; c++) {
    Serial.print(MJ_CLASSES[c]);
    Serial.print("=");
    Serial.print(lastScores[c], 3);

    if (c < 3) Serial.print(" | ");
  }
  Serial.println();

  if (lastPrediction == 3) {
    Serial.println("\nWORTHY");
    Serial.println("The hammer has accepted you.");
  } else {
    Serial.println("\nNOT WORTHY");
    Serial.println("Thor has declined your application.");
  }

  Serial.println("==========================================");
}

void handleStatus() {
  String json = "{";

  json += "\"evaluated\":";
  json += hasResult ? "true" : "false";

  json += ",\"busy\":";
  json += evaluationRequested ? "true" : "false";

  json += ",\"motion\":\"";
  json += MJ_CLASSES[lastPrediction];
  json += "\"";

  json += ",\"confidence\":";
  json += String(lastConfidence, 1);

  json += ",\"worthy\":";
  json += lastPrediction == 3 ? "true" : "false";

  json += ",\"score\":";
  json += lastPrediction == 3 ? "100" : "0";

  json += ",\"message\":\"";
  json += lastPrediction == 3
    ? "The hammer has accepted you."
    : "Thor has declined your application.";
  json += "\"";

  json += "}";

  server.send(200, "application/json", json);
}

void handleEvaluate() {
  if (evaluationRequested) {
    server.send(409, "application/json", "{\"started\":false}");
    return;
  }

  evaluationRequested = true;
  server.send(202, "application/json", "{\"started\":true}");
}

void setupWebServer() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("MJOLNIR", "worthy123");

  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", MJOLNIR_WEB_PAGE);
  });

  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/evaluate", HTTP_POST, handleEvaluate);

  server.begin();

  Serial.println();
  Serial.println("===== ASGARDIAN NETWORK =====");
  Serial.println("Network : MJOLNIR");
  Serial.println("Password: worthy123");
  Serial.print("Portal  : http://");
  Serial.println(WiFi.softAPIP());
  Serial.println("==============================");
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found. Check wiring.");
    while (true) {
      delay(1000);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  setupWebServer();

  Serial.println("MJOLNIR TinyML evaluator ready.");
  Serial.println("Press the web button or send any Serial character.");
}

void loop() {
  server.handleClient();

  if (evaluationRequested) {
    evaluateHammer();
  }

  if (Serial.available()) {
    while (Serial.available()) {
      Serial.read();
    }

    if (!evaluationRequested) {
      evaluationRequested = true;
    }
  }
}
