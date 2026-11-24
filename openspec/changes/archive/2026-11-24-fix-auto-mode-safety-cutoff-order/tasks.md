# Tasks

## 1. Control Loop Safety Ordering

- [x] 1.1 Move the hard software safety cutoff check in `loop()` in `src/main.cpp` ahead of the `if (autoControlEnabled)` closed-loop control block (after serial manual override handling), so reaching `adc >= MAX_SAFE_ADC` while `Forward` or `adc <= MIN_SAFE_ADC` while `Reverse` logs `"Reached MAX/MIN safe limit! Stopping."` and stops the motor in both automatic and manual modes; verify by inspecting `src/main.cpp` control flow and compiling `pio run -e esp32-c6-devkitc-1`.

## 2. Verification

- [x] 2.1 Run `pio test -e native -f test_motor` and `pio test -e native -f test_potentiometer` to confirm all existing native unit tests continue to pass with zero regressions.
