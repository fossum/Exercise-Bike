# Spec Delta

## Purpose

Decodes physical rotary encoder quadrature signals and an optional push button into a bounded, bounce-filtered discrete resistance level with edge-triggered change detection, independent of board-specific pin assignments.

## ADDED Requirements

### Requirement: Interrupt-driven quadrature decoding with bounce rejection
The rotary encoder input capability SHALL configure Phase A and Phase B as pull-up inputs, sample their initial state on initialization, handle state-change interrupts on both phases, and decode `(previous, current)` 2-bit transitions through a quadrature lookup table that produces `+1` or `-1` for valid single-bit steps and ignores invalid multi-bit bounce jumps.

#### Scenario: Decoding valid quadrature rotation
- **WHEN** Phase A and Phase B transition sequentially through valid single-bit Gray-code states in the clockwise or counter-clockwise direction
- **THEN** each valid transition accumulates a `+1` or `-1` sub-step delta corresponding to the rotation direction

#### Scenario: Rejecting invalid contact-bounce transitions
- **WHEN** both Phase A and Phase B appear to change simultaneously (an invalid two-bit transition such as `00 -> 11` or `01 -> 10`) or remain unchanged
- **THEN** the transition produces a `0` delta and does not alter the accumulated step count or level

### Requirement: Configurable detent step accumulation
The rotary encoder input capability SHALL accumulate valid sub-step deltas until the magnitude reaches a configured steps-per-click threshold (defaulting to `1` and coercing non-positive values to `1`), then reset the sub-step accumulator to `0` and step the level by `+1` or `-1`.

#### Scenario: Advancing one level after configured steps per click
- **WHEN** the encoder is configured with `stepsPerClick = 4` and receives 3 valid positive transitions followed by a 4th positive transition
- **THEN** the level remains unchanged after 3 transitions, increments by `1` on the 4th transition, and resets the sub-step accumulator to `0`

#### Scenario: Non-positive steps-per-click defaults to one
- **WHEN** the encoder is configured with `stepsPerClick <= 0`
- **THEN** each single valid quadrature transition steps the level by `1`

### Requirement: Clamped discrete level bounds
The rotary encoder input capability SHALL initialize its discrete level to `minLevel` and clamp all rotation updates within `[minLevel, maxLevel]` (default `1..32`), refusing to emit a level-changed event when rotation attempts to move past an endpoint.

#### Scenario: Clamping at minimum and maximum levels
- **WHEN** the level is at `maxLevel` and a positive detent click occurs, or at `minLevel` and a negative detent click occurs
- **THEN** the level remains at that boundary and the change flag is not set

### Requirement: Consuming change detection
The rotary encoder input capability SHALL latch a change flag whenever the discrete level changes and clear that flag when queried so downstream consumers observe each change once.

#### Scenario: Reading and clearing a pending level change
- **WHEN** a rotation changes the level and the caller checks for changes twice in a row with no further rotation
- **THEN** the first check returns `true` and the second check returns `false`

### Requirement: Atomic programmatic level override
The rotary encoder input capability SHALL allow callers to set the level programmatically, clamping the requested value to `[minLevel, maxLevel]`, resetting any partial sub-step accumulation to `0`, setting the change flag, and applying the update atomically with interrupts disabled.

#### Scenario: Setting level within and outside bounds
- **WHEN** the level is programmatically set to a value above `maxLevel` or below `minLevel`
- **THEN** the stored level clamps to `maxLevel` or `minLevel` respectively, the sub-step accumulator resets to `0`, and the change flag becomes `true`

### Requirement: Optional debounced push-button input
The rotary encoder input capability SHALL support an optional active-low push-button input (disabled when configured with a negative channel identifier) that reports a press once per `HIGH -> LOW` transition separated by more than `200 ms` from the previous accepted press and requires the button to return `HIGH` before re-triggering.

#### Scenario: Debounced single-trigger button press
- **WHEN** a push button is configured, transitions from `HIGH` to `LOW` more than `200 ms` after any prior press, and remains held `LOW`
- **THEN** the press check returns `true` once on the initial press and `false` on subsequent checks until the button returns `HIGH` and is pressed again after `200 ms`

#### Scenario: Disabled push button never triggers
- **WHEN** no push-button input is configured (negative identifier)
- **THEN** checking for a button press always returns `false`
