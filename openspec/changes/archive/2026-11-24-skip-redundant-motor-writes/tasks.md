# Tasks

## 1. Motor Output Deduplication and Unit Tests

- [x] 1.1 Update `Motor::begin()` in `src/motor.cpp` to set `direction_ = MotorDirection::Stopped`, `speedPercent_ = 0.0f`, and call `applyOutput()` directly so all configured direction, PWM, and standby pins are unconditionally initialized, and verify initialization assertions pass with `pio test -e native -f test_motor`.
- [x] 1.2 Add state-change guards in `Motor::forward()`, `Motor::reverse()`, `Motor::stop()`, and `Motor::setSpeedPercent()` in `src/motor.cpp` to return early without calling `applyOutput()` when effective `(direction_, clamped speedPercent_)` is unchanged, and verify existing unit tests pass with `pio test -e native -f test_motor`.
- [x] 1.3 Add unit test(s) in `test/test_motor/test_main.cpp` checking `digitalWriteCallCount` and `analogWriteCallCount` to verify that `begin()` writes outputs unconditionally, redundant `forward()`, `reverse()`, `stop()`, `setSpeedPercent()` (including clamped-equivalent speeds and calls while stopped), and `setModeChar()` calls emit zero hardware writes, and actual direction or speed changes still write outputs; verify all tests pass with `pio test -e native -f test_motor`.

## 2. Full Verification

- [x] 2.1 Run the full native test suite (`pio test -e native`) and compile the ESP32-C6 firmware target (`pio run -e esp32-c6-devkitc-1`) to confirm clean compilation and zero regressions across all modules.
