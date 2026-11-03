# Desktop tests and debugging

Tests compile the firmware with simulated Arduino/I2S hardware and time. No board
is required; ESP8266 execution speed and DMA behavior are not simulated.

## Run tests on Linux

Install prerequisites (Ubuntu/Debian):

```bash
sudo apt update
sudo apt install build-essential cmake gdb
```

From the repository root:

```bash
cmake -S . -B build/desktop -DCMAKE_BUILD_TYPE=Debug
cmake --build build/desktop --config Debug
ctest --test-dir build/desktop -C Debug --output-on-failure
```

Tests cover setup, channel selection, detection, output timing, cooldown,
retriggering, timer wraparound and failed reads across three firmware variants.

## Debug in VS Code on Linux

1. Install the Microsoft C/C++ extension and open this repository folder.
2. Set a breakpoint in `loop()` or `detectPistolShot()` in `src/main.cpp`.
3. Select **Debug firmware unit tests (Linux)** in Run and Debug, then press
   **F5**. The configuration builds the tests automatically.
4. Use **F10** to step over, **F11** to step into, and **F5** to continue.

Watch `fake::output`, `fake::shots`, `noise_floor`, `prev_level` and `cooldown`.
Run all variants via **Terminal > Run Task > Run desktop tests**.

To debug one scenario, change `args` in `.vscode/launch.json`:

```json
"args": ["immediate_detection_and_timeout"]
```

Use `"args": []` to run all scenarios. The same scenario can run from a terminal:

```bash
./build/desktop/firmware_tests immediate_detection_and_timeout
```

The default executable is `firmware_tests`. Change the launch configuration's
`program` filename to `firmware_long_fir_tests` or `arduino_firmware_tests` for
another variant. For the Arduino variant, set breakpoints in
`arduino/starter-pistol/starter-pistol.ino`.

## Change simulated input

Edit `tests/firmware_test.cpp`. The helper `frame(left, index, right)` supplies
samples and sets simulated time using a 16 kHz frame index. Hardware fakes are in
`tests/fakes/`.
