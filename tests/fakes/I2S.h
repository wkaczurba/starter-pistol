#pragma once
#include "Arduino.h"
inline void i2s_rxtx_begin(bool rx, bool tx) { fake::rx = rx; fake::tx = tx; }
inline void i2s_set_rate(unsigned rate) { fake::sample_rate = rate; }
inline bool i2s_read_sample(int16_t* left, int16_t* right, bool blocking) {
  ++fake::reads;
  fake::blocking = blocking;
  if (!fake::read_ok) return false;
  *left = fake::left;
  *right = fake::right;
  return true;
}
