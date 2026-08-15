# starter-pistol

PlatformIO firmware template for a NodeMCU ESP-12E (ESP8266) board that
listens on an I2S microphone for a starter-pistol shot and fires an external
MOSFET-driven output when one is detected.

## Hardware

| Signal          | NodeMCU pin | Notes                          |
|-----------------|-------------|---------------------------------|
| MOSFET gate     | `D0`        | Output, active-high             |

Pin assignments live in [`include/pins.h`](include/pins.h).

## Firmware

- `Serial` at 115200 baud for logging.
- I2S RX started at 16 kHz (`i2s_rxtx_begin` / `i2s_set_rate`) for the mic.
- Detection pipeline (FIR filter + noise-floor spike detector) ported as-is
  from `ESP8266-BUG-I2S-MIC` — see [`src/main.cpp`](src/main.cpp).
- No WiFi or UDP: this build only drives the local MOSFET output, it does not
  stream audio or telemetry over the network.

## Build

```bash
pio run
pio run -t upload
pio device monitor -b 115200
```
