# Tasks

## 1. Button Edge-State Tracking

- [x] 1.1 Update `RotaryEncoder::wasButtonPressed()` in `src/rotary_encoder.cpp` so `_lastButtonState` is always updated to `reading` on every call (including when `reading == LOW` inside the `200 ms` debounce window), while only returning `true` and updating `_lastButtonPressMs` on a `HIGH -> LOW` transition with `(now - _lastButtonPressMs > 200)`; verify by inspecting `src/rotary_encoder.cpp` and compiling `pio run -e esp32-c6-devkitc-1`.

## 2. Verification

- [x] 2.1 Run `pio test -e native -f test_motor` and `pio test -e native -f test_potentiometer` to confirm existing native unit tests pass with zero regressions.
