# Proposal

## Why

The resistance control loop invokes motor commands (`forward`, `reverse`, `stop`) on every 10 ms iteration even when the motor is already in the requested direction and speed, causing redundant GPIO and PWM output writes at 100 Hz. Guarding output application against unchanged state eliminates unnecessary hardware writes and potential PWM re-initialization glitches while keeping startup pin initialization deterministic.

## What Changes

- Skip invoking `applyOutput()` in `Motor::forward()`, `Motor::reverse()`, `Motor::stop()`, and `Motor::setSpeedPercent()` (and by delegation `Motor::setModeChar()`) when both effective direction (`direction_`) and effective clamped speed (`speedPercent_`) are unchanged.
- Preserve unconditional hardware output initialization in `Motor::begin()` so all configured direction, PWM, and standby outputs are always driven to the safe `Stopped` state on startup even though member fields start in `Stopped` / `0%`.
- Add native unit tests verifying that redundant commands emit zero digital or analog hardware writes while state transitions and `begin()` emit the expected writes.

## Capabilities

### New Capabilities

<!-- None -->

### Modified Capabilities

- `motor-driver`: Require suppressing redundant hardware output writes when direction and clamped speed do not change, while preserving unconditional hardware output writes during initialization.

## Impact

- **Firmware**: `src/motor.cpp` (state-change checks in `forward`, `reverse`, `stop`, `setSpeedPercent`; direct initialization output in `begin`). No public API changes in `include/motor.h`.
- **Tests**: `test/test_motor/test_main.cpp` (new unit test coverage using existing mock write counters).
- **Specs**: Delta spec under `openspec/changes/skip-redundant-motor-writes/specs/motor-driver/spec.md`.
