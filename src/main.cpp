#include <Arduino.h>
#include <I2S.h>

#include "pins.h"

// ---------------------------------------------------------------------------
// I2S microphone configuration
// ---------------------------------------------------------------------------
#define SAMPLE_RATE 16000
#define I2S_READ_BATCH 100 // samples processed per loop() pass

// ---------------------------------------------------------------------------
// Shot detection tuning (FIR + noise-floor spike detector).
// ---------------------------------------------------------------------------
#define MIN_SPIKE_LEVEL 12000.0f
#define SPIKE_RATIO 18.0f
#define SHARP_RISE 8000.0f
#define NOISE_ALPHA 0.0002f
#define COOLDOWN_SAMPLES 3200 // 200 ms at 16 kHz

#ifdef USE_LONG_FIR
#define FIR_LEN 63
// 3-6 kHz band-pass FIR, fs=16 kHz
static const float h[FIR_LEN] = {
    -0.000000000f, -0.001022169f, -0.000000000f,  0.001808807f,
     0.000000000f, -0.003067968f, -0.000000000f,  0.004823591f,
     0.000000000f, -0.007093902f, -0.000000000f,  0.009894618f,
     0.000000000f, -0.013236624f, -0.000000000f,  0.017120627f,
     0.000000000f, -0.021536681f, -0.000000000f,  0.026464321f,
     0.000000000f, -0.031873220f, -0.000000000f,  0.037724560f,
     0.000000000f, -0.043971060f, -0.000000000f,  0.050558636f,
     0.000000000f, -0.057428697f, -0.000000000f,  0.314528000f,
    -0.000000000f, -0.057428697f,  0.000000000f,  0.050558636f,
    -0.000000000f, -0.043971060f,  0.000000000f,  0.037724560f,
    -0.000000000f, -0.031873220f,  0.000000000f,  0.026464321f,
    -0.000000000f, -0.021536681f,  0.000000000f,  0.017120627f,
    -0.000000000f, -0.013236624f,  0.000000000f,  0.009894618f,
    -0.000000000f, -0.007093902f,  0.000000000f,  0.004823591f,
    -0.000000000f, -0.003067968f,  0.000000000f,  0.001808807f,
    -0.000000000f, -0.001022169f, -0.000000000f
};
#else
#define FIR_LEN 7
static const float h[FIR_LEN] = {
   -0.06f,
    0.00f,
    0.24f,
    0.64f,
    0.24f,
    0.00f,
   -0.06f
};
#endif

static float hist[FIR_LEN] = {0};
static int fir_pos = 0;

static float noise_floor = 500.0f;
static float prev_level = 0.0f;
static int cooldown = 0;

// ---------------------------------------------------------------------------
// MOSFET trigger output (D0)
// ---------------------------------------------------------------------------
#define MOSFET_ON_TIME_MS 1000

bool mosfetActive = false;
uint32_t mosfetOffTime = 0;

static void mosfet_off() {
  digitalWrite(PIN_MOSFET, LOW);
}

static void mosfet_on() {
  digitalWrite(PIN_MOSFET, HIGH);
}

static float fir_process(int16_t sample) {
  hist[fir_pos] = (float)sample;

  float y = 0.0f;
  int idx = fir_pos;

  for (int i = 0; i < FIR_LEN; i++) {
    y += h[i] * hist[idx];
    idx--;
    if (idx < 0) idx = FIR_LEN - 1;
  }

  fir_pos++;
  if (fir_pos >= FIR_LEN) fir_pos = 0;

  return y;
}

static void onPistolShot() {
  mosfet_on();
  mosfetOffTime = millis() + MOSFET_ON_TIME_MS;
  mosfetActive = true;
  Serial.print("!");
}

static void detectPistolShot(float filtered) {
  float level = fabs(filtered);
  float rise = level - prev_level;
  prev_level = level;

  if (level < noise_floor * 3.0f) {
    noise_floor = (1.0f - NOISE_ALPHA) * noise_floor + NOISE_ALPHA * level;
  }

  if (cooldown > 0) {
    cooldown--;
    return;
  }

  bool loud_enough = level > MIN_SPIKE_LEVEL;
  bool above_noise = level > noise_floor * SPIKE_RATIO;
  bool sharp_rise  = rise > SHARP_RISE;

  if (loud_enough && above_noise && sharp_rise) {
    onPistolShot();
    cooldown = COOLDOWN_SAMPLES;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_MOSFET, OUTPUT);
  mosfet_off();

  i2s_rxtx_begin(true, false);
  i2s_set_rate(SAMPLE_RATE);
}

void loop() {
  // House-keeping: release the MOSFET once its on-time has elapsed.
  if (mosfetActive && (int32_t)(millis() - mosfetOffTime) >= 0) {
    mosfet_off();
    mosfetActive = false;
  }

  for (int i = 0; i < I2S_READ_BATCH; i++) {
    int16_t left, right;
    i2s_read_sample(&left, &right, true);

    // Filter only the useful mic channel, as in the reference sketch.
    float left_filtered = fir_process(left);
    float right_unfiltered = left; // reference takes detection input from left too.

    detectPistolShot(right_unfiltered);
    (void)left_filtered;
  }
}
