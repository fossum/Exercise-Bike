# Proposal

## Why

OpenSpec was recently initialized in this repository without any durable capability specs, leaving the Exercise-Bike firmware's implemented behavior undocumented as formal requirements. Establishing baseline specs for all currently active features creates a clear source of truth that future firmware changes and hardware specs can build upon.

## What Changes

- Document the currently active Exercise-Bike firmware behavior as four baseline capability specifications reflecting the current branch (`feature/pinout-changes-and-standby-addon`, including PWM speed control and H-bridge standby support).
- Omit board-specific GPIO pin assignments from these behavioral specifications; physical pin mappings are out of scope here and reserved for a separate PCB/hardware specification.
- Exclude unimplemented planned features (such as cadence sensing) so the baseline strictly reflects active capabilities.
- Make no changes to firmware source code or unit tests.

## Capabilities

### New Capabilities

- `potentiometer-sensing`: Multi-sample averaged ADC and millivolt reading, voltage and ADC range reporting, min/max raw calibration with clamped normalized/percentage outputs, and injectable hardware readers for native testing.
- `motor-driver`: H-bridge direction control (`Forward`, `Reverse`, `Stopped`), optional PWM speed scaling and clamping, optional standby (`STBY`) output sequencing, single-character mode commands (`f`, `r`, `s`), and injectable hardware writers for native testing.
- `rotary-encoder-input`: Interrupt-driven quadrature decoding with lookup-table bounce rejection, configurable detent step accumulation, clamped discrete level state, change-flag consumption, atomic programmatic level setting, and optional debounced push-button input.
- `resistance-control`: Startup potentiometer-to-level synchronization, linear level-to-ADC closed-loop control with deadband, serial manual override with automatic recovery on encoder turn, hard software end-stop safety cutoffs, and periodic serial telemetry.

### Modified Capabilities

<!-- None: no existing specs exist yet. -->

## Impact

- **Specs**: Introduces four new capability specs under `openspec/changes/baseline-active-features/specs/`, ready to be synced into `openspec/specs/`.
- **Code / Hardware**: No firmware source, test, or pinout changes. Existing source files (`src/main.cpp`, `src/potentiometer.cpp`, `src/motor.cpp`, `src/rotary_encoder.cpp`, and headers in `include/`) serve as the behavioral reference.
