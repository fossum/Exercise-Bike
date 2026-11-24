# Proposal

## Why

On the physical exercise bike hardware, the resistance mechanism bottoms out at a raw potentiometer ADC reading of `1920`. Raising `MAX_SAFE_ADC` to `3000` drives the motor past the physical travel limit, so lowering the upper bound to `1920` prevents mechanical binding and aligns discrete resistance levels `1..32` with the actual usable travel range (`400..1920`).

## What Changes

- Lower `MAX_SAFE_ADC` in `src/main.cpp` from `3000` to `1920`, updating startup clamping/mapping, potentiometer calibration bounds, level-to-ADC mapping (`1..32` -> `400..1920`), closed-loop forward travel limit, and hard safety cutoff.
- Update `docs/pinouts.md` to document the safe raw ADC range as `400–1920`.
- Update the `resistance-control` specification to reflect the `400..1920` safe ADC window across startup synchronization, closed-loop mapping, and hard software travel limits.

## Capabilities

### New Capabilities

<!-- None -->

### Modified Capabilities

- `resistance-control`: Change the upper safe potentiometer ADC limit from `3000` to `1920` for startup position sync, level-to-ADC closed-loop mapping (`1..32` -> `400..1920`), and hard software end-stop cutoffs.

## Impact

- **Firmware**: `src/main.cpp` (`MAX_SAFE_ADC` constant).
- **Documentation**: `docs/pinouts.md` (safe raw ADC range note).
- **Specs**: Delta spec under `openspec/changes/update-max-safe-adc-1920/specs/resistance-control/spec.md`.
