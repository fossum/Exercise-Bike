# Spec Delta

## ADDED Requirements

### Requirement: Suppress redundant hardware output writes when state is unchanged
The motor driver capability SHALL skip writing direction, PWM, and standby hardware outputs whenever a directional command, stop command, speed update, or mode-character command produces the same effective direction and clamped speed percentage as the current state, while preserving unconditional hardware output writes during initialization.

#### Scenario: Initialization writes hardware outputs unconditionally
- **WHEN** the motor driver is initialized from its default `Stopped` (`0%`) state
- **THEN** all configured direction, optional PWM, and optional standby channels are driven to their safe `Stopped` outputs even though the logical state was already `Stopped` at `0%`

#### Scenario: Redundant directional commands skip hardware writes
- **WHEN** the motor is already running `Forward` at `80.0%` and receives another `Forward` command at `80.0%` (or already running `Reverse` at `50.0%` and receives `Reverse` at `50.0%`)
- **THEN** the motor remains in that state and performs zero digital and zero analog output writes on the redundant command

#### Scenario: Redundant stop and stopped-state commands skip hardware writes
- **WHEN** the motor is already `Stopped` (after initialization) and receives `Stopped`, a non-positive speed `Forward`/`Reverse` command, or a speed update while stopped
- **THEN** the motor remains `Stopped` at `0%` and performs zero digital and zero analog output writes

#### Scenario: Clamped-equivalent speed updates skip hardware writes
- **WHEN** the motor is running `Forward` at `100.0%` and receives a speed update or `Forward` command with `100.0%` or `150.0%` (which clamps to `100.0%`)
- **THEN** the motor remains `Forward` at `100.0%` and performs zero digital and zero analog output writes

#### Scenario: Redundant mode-character commands skip hardware writes
- **WHEN** the motor receives `'f'` at `75.0%` twice in a row (or `'s'` while already `Stopped`)
- **THEN** hardware outputs are written only on the first transition and skipped on the repeated command

#### Scenario: Actual direction or speed changes still write hardware outputs
- **WHEN** the motor is running `Forward` at `50.0%` and changes speed to `80.0%`, or changes direction to `Reverse` at `80.0%`, or transitions to `Stopped`
- **THEN** hardware direction, optional PWM, and optional standby outputs are written for each state transition
