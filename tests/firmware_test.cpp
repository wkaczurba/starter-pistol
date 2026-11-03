#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

// Compile the actual firmware into this translation unit. Only hardware APIs
// are replaced, so breakpoints in setup(), loop(), and the detector work.
#ifdef TEST_ARDUINO_SKETCH
#include "../arduino/starter-pistol/starter-pistol.ino"
#else
#include "../src/main.cpp"
#endif

static void check(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

static void reset() {
  fake::time_us = 0;
  fake::output = HIGH;
  fake::shots = fake::reads = 0;
  fake::left = fake::right = 0;
  fake::read_ok = true;
  std::fill(std::begin(hist), std::end(hist), 0.0f);
  fir_pos = 0;
  noise_floor = 500.0f;
  prev_level = 0.0f;
  cooldown = 0;
  mosfetActive = false;
  mosfetOffTime = 0;
  setup();
}

// Integer frame counts avoid rounding 62.5 microseconds to an integer per frame.
static void frame(int16_t left, uint64_t index, int16_t right = 0) {
  fake::time_us = index * 1000000ULL / SAMPLE_RATE;
  fake::left = left;
  fake::right = right;
  loop();
}

static void setup_and_one_frame() {
  check(fake::output == LOW, "setup must switch output off");
  check(fake::output_pin == PIN_MOSFET && fake::output_mode == OUTPUT,
        "setup must configure MOSFET pin");
  check(fake::rx && !fake::tx && fake::sample_rate == 16000,
        "setup must configure microphone reception");
  frame(0, 0);
  check(fake::reads == 1 && fake::blocking, "loop must read exactly one frame");
}

static void quiet_and_right_channel() {
  for (uint64_t i = 0; i < 16000; ++i) frame(i % 2 ? 400 : -400, i, 30000);
  check(fake::shots == 0 && fake::output == LOW,
        "quiet left input and loud unused right input must not trigger");
}

static void immediate_detection_and_timeout() {
  frame(20000, 0);
  check(fake::shots == 1 && fake::output == HIGH,
        "shot must activate output on the same frame");
  frame(0, 15999);
  check(fake::output == HIGH, "output must remain on before deadline");
  frame(0, 16000);
  check(fake::output == LOW && !mosfetActive, "output must turn off at deadline");
}

static void cooldown_and_retrigger() {
  frame(20000, 0);
  for (uint64_t i = 1; i <= 3200; ++i) frame(i % 2 ? 0 : 20000, i);
  check(fake::shots == 1, "shots during 3200-frame cooldown must be ignored");
  frame(0, 3201);
  frame(20000, 3202);
  check(fake::shots == 2, "shot after cooldown must trigger again");
  frame(0, 16000);
  check(fake::output == HIGH, "retrigger must extend output deadline");
  frame(0, 19200); // retrigger at millis() == 200, deadline == 1200
  check(fake::output == LOW, "extended deadline must turn output off");
}

static void millis_wraparound() {
  fake::time_us = uint64_t(std::numeric_limits<uint32_t>::max() - 499) * 1000;
  fake::left = 20000;
  loop();
  fake::left = 0;
  fake::time_us += 999000;
  loop();
  check(fake::output == HIGH, "wrapped deadline must not expire early");
  fake::time_us += 1000;
  loop();
  check(fake::output == LOW, "deadline must work across millis wraparound");
}

static void failed_read() {
  fake::read_ok = false;
  loop();
  check(fake::shots == 0 && fir_pos == 0,
        "failed read must not process uninitialized samples");
}

int main(int argc, char** argv) {
  struct Test { const char* name; void (*run)(); };
  const Test tests[] = {
    {"setup_and_one_frame", setup_and_one_frame},
    {"quiet_and_right_channel", quiet_and_right_channel},
    {"immediate_detection_and_timeout", immediate_detection_and_timeout},
    {"cooldown_and_retrigger", cooldown_and_retrigger},
    {"millis_wraparound", millis_wraparound},
    {"failed_read", failed_read},
  };
  unsigned selected = 0;
  unsigned failures = 0;
  for (const auto& test : tests) {
    if (argc > 1 && std::string(argv[1]) != test.name) continue;
    ++selected;
    try {
      reset();
      test.run();
      std::cout << "PASS " << test.name << '\n';
    } catch (const std::exception& error) {
      ++failures;
      std::cerr << "FAIL " << test.name << ": " << error.what() << '\n';
    }
  }
  if (!selected) std::cerr << "Unknown test name\n";
  return selected && !failures ? EXIT_SUCCESS : EXIT_FAILURE;
}
