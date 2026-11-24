# Tasks

## 1. Firmware & Documentation Calibration Update

- [x] 1.1 Update `MAX_SAFE_ADC` from `3000` to `1920` in `src/main.cpp` and verify clean compilation with `pio run -e esp32-c6-devkitc-1`.
- [x] 1.2 Update the safe raw ADC range note in `docs/pinouts.md` from `400–3000` to `400–1920` and verify no stale `3000` safe-limit references remain in `src/` or `docs/`.

## 2. Verification

- [x] 2.1 Run `pio test -e native -f test_motor` and `pio test -e native -f test_potentiometer` to confirm all existing native unit tests pass with zero regressions.
