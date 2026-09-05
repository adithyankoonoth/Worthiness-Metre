#pragma once

// TinyML parameters for the ESP8266 firmware.
// Feature order: az_max, gyromag_mean, accmag_mean, gz_std,
// gyromag_max, az_mean, gz_range, gz_min

static const float MJ_FEATURE_MEAN[8] = {
  5.153500f, 2.226221f, 11.267284f, 0.916112f,
  3.949313f, 1.923560f, 3.367538f, -1.791275f
};

static const float MJ_FEATURE_STD[8] = {
  4.268845f, 1.497612f, 2.138818f, 1.315470f,
  2.476936f, 3.093634f, 3.799546f, 2.039167f
};

static const float MJ_COEF[4][8] = {
  {-0.904024f,-0.904239f,-0.547212f,-0.289198f,-1.083357f,-0.251066f,-0.430242f, 0.411445f},
  { 0.992082f,-0.212778f,-1.347607f,-0.263218f, 0.130431f, 0.135725f,-0.501063f,-0.012349f},
  { 0.268017f,-0.714703f, 1.846837f, 1.011618f,-0.167863f, 0.026859f, 1.001705f,-0.427299f},
  {-0.356075f, 1.831720f, 0.047982f,-0.459202f, 1.120789f, 0.088482f,-0.070400f, 0.028203f}
};

static const float MJ_BIAS[4] = {
  -1.378618f, 0.764290f, 0.193740f, 0.420588f
};

static const char* MJ_CLASSES[4] = {"IDLE", "LIFT", "SHAKE", "SWING"};
