# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

ESP32-S3 firmware (PlatformIO + Arduino framework) that drives a small SPI TFT display (LovyanGFX / ST7789, 128x128) showing power/battery status pulled live from a Home Assistant instance over its WebSocket API. It also reads a physical grid-power sensor pin and reports its state back to Home Assistant.

## Build / Common Commands

This is a PlatformIO project (not npm/make). Use the `pio` CLI:

- Build: `pio run`
- Upload to device: `pio run -t upload`
- Serial monitor (115200 baud): `pio device monitor`
- Build + upload + monitor in one go: `pio run -t upload -t monitor`
- Clean: `pio run -t clean`

There is no test suite (`test/` is the empty PlatformIO placeholder directory) and no lint step configured.

### Required local setup

`src/secrets.h` is gitignored and must be created from `src/secrets.h.example` before building — it defines `WIFI_SSID`, `WIFI_PASSWORD`, `HA_HOST`, `HA_PORT`, `HA_TOKEN` (a Home Assistant long-lived access token). The build will fail without this file.

### Target board

Defined in `platformio.ini` under `[env:esp32-s3-devkitc-1]` (`espressif32` platform, `arduino` framework, 4MB flash, PSRAM enabled, USB CDC on boot). Note: `.pio/build/` and `build/` contain leftover artifacts from other boards used earlier in this project's history (ESP32-S3-Zero, upesy_wroom) — the only board currently defined in `platformio.ini` is `esp32-s3-devkitc-1`. Don't assume those other env names are still buildable without adding them back to `platformio.ini`.

## Architecture

### Data flow

```
Home Assistant  <--WebSocket-->  WebSocketClient (websocket.cpp)
                                        |
                                        v
                                  Controller (diffs old vs new State)
                                        |
                                        v
                              ScreenComponent subclasses (draw to LGFX)

Grid power sensor pin (GPIO_NUM_13) --> main.cpp checkPower() --> Controller --> WebSocketClient (reports sensor state back to HA as an event)
```

- **`State` (`state.h`)**: plain data holder for one snapshot of everything shown on screen (WiFi signal, websocket connection/auth, power availability, battery %, charging power, charge/discharge durations). `calculateState()` derives a tri-state `STATE_NORMAL` / `STATE_CHARGING` / `STATE_DISCHARGING` from actual battery flow (discharge power > 0 → discharging, charge power > 0 → charging), falling back to grid availability. `getPowerMetric()` / `getTimeMetric()` pick which underlying value is "the" metric to display based on that state — components don't do this branching themselves.
- **`Controller` (`controller.cpp`)**: owns the `lastKnownState` and is the single point where a new `State` is diffed against the old one; each screen component is only told to redraw when its specific slice of state actually changed (see `onStateUpdate`). This is a hand-rolled dirty-checking layer — when adding a new displayed value, wire it through here rather than having components poll or having callers push to components directly.
- **`ScreenComponent` (`components/screen_component.*`)**: base class all display widgets inherit from (`Battery`, `PowerStatus`, `PowerMetric`, `TimeMetric`, `WiFiStatus`, `WSStatus`). Holds its own `LGFX` display copy, an `(x, y)` location, and a frame-rate/frame-counter helper (`getFrameNumber()`) used for blink-style animations (e.g. the battery charging fill blinks the next segment). Each subclass implements `initialDraw()` (draw static chrome once) and `draw()` (redraw only what changed) — this split matters, don't collapse them.
- **`WebSocketClient` (`websocket.cpp`)**: wraps `esp_websocket_client`. Handles HA's auth handshake (`auth_required` → `sendAuth` → `auth_ok`/`auth_invalid`), then issues a `render_template` subscription (`sendRenderInitialData`) that bundles multiple HA entity states into one templated request (see the `SENSOR_*` constants at the top of the file for the exact entity IDs this firmware depends on). Incoming text frames are reassembled across chunks using `messageBuffer`/`expectedMessageLength` before being parsed as JSON (ArduinoJson) — `WEBSOCKET_EVENT_DATA` payloads can arrive fragmented, so don't assume a single event carries a complete message. It also pushes local sensor state (`grid_power_monitor_update`) back to HA whenever the physical sensor reading changes from what was last reported (`checkPowerSensor`).
- **`network.cpp`**: WiFi connect/reconnect loop (`initWiFi`, `checkWiFi`) with a timeout-triggered `ESP.restart()` if initial connection stalls. On WiFi drop it explicitly tears down and reinits the `WebSocketClient` rather than relying on the client's own reconnect.
- **`main.cpp`**: composition root. Constructs all components/controller as globals, wires them together in `setup()`, and the `loop()` is a simple tick: check power pin → redraw grid lines → check WiFi → `controller.draw()`. `debug` flag (compile-time bool) toggles drawing colored corner reference lines.
- **`LGFX_User_Setup.hpp`**: hardware pin mapping for the display (SPI bus + ST7789 panel config — SCLK/MOSI/DC/CS/RST pins, panel offsets/inversion). Change this file, not `main.cpp`, when the physical display wiring changes.

### Adding a new Home Assistant-backed metric

Touch points, in order: add the entity ID constant and template line in `websocket.cpp` (`sendRenderInitialData`), read it out in `handleState()` into the `State`, expose any derived logic on `State` itself (not in the controller or component), diff it in `Controller::onStateUpdate`, and add/extend a `ScreenComponent` subclass to render it.
