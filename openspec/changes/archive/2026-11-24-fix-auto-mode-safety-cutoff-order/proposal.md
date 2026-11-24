# Proposal

## Why

The `resistance-control` specification requires enforcing hard software end-stops in both automatic and manual modes by immediately stopping the motor and logging a limit warning when running `Forward` at `adc >= 3000` or `Reverse` at `adc <= 400`. In `src/main.cpp`, the automatic control block currently runs before the safety cutoff check and calls `motor.stop()` in its `else` branch, preventing the safety limit warning from firing during automatic mode.

## What Changes

- Reorder `loop()` in `src/main.cpp` so the hard software safety cutoff check runs **before** the automatic closed-loop control block (and after serial manual override handling), ensuring `motor.getDirection()` is still `Forward` or `Reverse` when `adc` reaches `>= 3000` or `<= 400` in both automatic and manual modes.
- Make no changes to `openspec/specs/resistance-control/spec.md` (`skip_specs: true` in `.openspec.yaml`), as this aligns the implementation with the existing specification without changing spec-level requirements.

## Capabilities

### New Capabilities

<!-- None -->

### Modified Capabilities

<!-- None: the existing `resistance-control` specification already requires this behavior in both automatic and manual modes. -->

## Impact

- **Firmware**: `src/main.cpp` (`loop()` control-flow ordering only).
- **Specs**: None (`skip_specs: true`).
