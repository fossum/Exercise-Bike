# Proposal

## Why

The `rotary-encoder-input` specification requires reporting a button press once per `HIGH -> LOW` transition separated by more than `200 ms` from the previous accepted press. In `RotaryEncoder::wasButtonPressed()` (`src/rotary_encoder.cpp`), `_lastButtonState` is only updated to `LOW` when `(now - _lastButtonPressMs > 200)` is true, so pressing and holding the button within `200 ms` of boot or a previous press leaves `_lastButtonState == HIGH` and fires a spurious delayed press mid-hold as soon as `200 ms` elapses.

## What Changes

- Update `RotaryEncoder::wasButtonPressed()` in `src/rotary_encoder.cpp` so `_lastButtonState` always tracks `reading` on every call (whether or not the `200 ms` debounce window has elapsed), ensuring a press is only recognized on a genuine `HIGH -> LOW` edge outside the debounce window and never mid-hold.
- Make no changes to `openspec/specs/rotary-encoder-input/spec.md` (`skip_specs: true` in `.openspec.yaml`), as this aligns the implementation with the existing requirement.

## Capabilities

### New Capabilities

<!-- None -->

### Modified Capabilities

<!-- None: the existing `rotary-encoder-input` specification already requires a HIGH -> LOW transition separated by > 200 ms. -->

## Impact

- **Firmware**: `src/rotary_encoder.cpp` (`RotaryEncoder::wasButtonPressed()` state tracking only).
- **Specs**: None (`skip_specs: true`).
