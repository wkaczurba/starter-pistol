#pragma once
#include <cmath>
#include <cstdint>

constexpr int D0 = 16;
constexpr int OUTPUT = 1;
constexpr int LOW = 0;
constexpr int HIGH = 1;

namespace fake {
inline uint64_t time_us = 0;
inline int output = LOW;
inline int output_pin = -1;
inline int output_mode = -1;
inline unsigned shots = 0;
inline unsigned reads = 0;
inline int16_t left = 0;
inline int16_t right = 0;
inline bool read_ok = true;
inline bool blocking = false;
inline bool rx = false;
inline bool tx = false;
inline unsigned sample_rate = 0;
}

inline uint32_t millis() { return static_cast<uint32_t>(fake::time_us / 1000); }
inline void pinMode(int pin, int mode) {
  fake::output_pin = pin;
  fake::output_mode = mode;
}
inline void digitalWrite(int pin, int value) {
  fake::output_pin = pin;
  fake::output = value;
}
struct FakeSerial {
  void begin(unsigned) {}
  void print(const char* text) { if (text[0] == '!') ++fake::shots; }
  void print(int) {}
};
inline FakeSerial Serial;
