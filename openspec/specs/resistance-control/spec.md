# resistance-control Specification

## Purpose

Coordinates the potentiometer, motor driver, rotary encoder, and serial interface into a closed-loop exercise bike resistance controller with startup position sync, manual serial override, hard software travel limits, and status telemetry.

## Requirements

### Requirement: Startup position synchronization
The resistance controller SHALL read the current physical potentiometer position at startup, clamp it to the safe operating window (`400..3000` raw ADC), map it linearly to a discrete resistance level (`1..32`), synchronize the encoder level and target ADC to that initial level, and clear the initial encoder change event.

#### Scenario: Booting with potentiometer mid-travel
- **WHEN** the controller boots while the potentiometer reads `1700` raw ADC
- **THEN** the controller sets the initial resistance level to the corresponding mapped value in `1..32`, sets the target ADC to that level's mapped target, clears the pending change flag, and logs the startup banner with initial level, pot ADC, and target ADC

#### Scenario: Booting with potentiometer outside safe window
- **WHEN** the controller boots while the potentiometer reads below `400` or above `3000` raw ADC
- **THEN** the startup mapping clamps the reading to `400` (level `1`) or `3000` (level `32`) respectively

### Requirement: Closed-loop level-to-position control
The resistance controller SHALL map resistance levels `1..32` linearly to target ADC positions `400..3000` and evaluate position error (`targetADC - adc`) on a `10 ms` control loop when automatic control is active, driving the motor at `80%` speed toward the target and stopping within a `+/- 25` ADC deadband.

#### Scenario: Driving forward toward a higher resistance level
- **WHEN** automatic control is active, `targetADC - adc > 25`, and `adc < 3000`
- **THEN** the controller drives the motor `Forward` at `80%` speed to increase the potentiometer ADC reading

#### Scenario: Driving reverse toward a lower resistance level
- **WHEN** automatic control is active, `targetADC - adc < -25`, and `adc > 400`
- **THEN** the controller drives the motor `Reverse` at `80%` speed to decrease the potentiometer ADC reading

#### Scenario: Stopping inside the deadband window
- **WHEN** automatic control is active and `|targetADC - adc| <= 25`
- **THEN** the controller stops the motor to prevent oscillation and hunting

### Requirement: Serial manual override and automatic resumption
The resistance controller SHALL accept `'f'`, `'r'`, and `'s'` commands over the `115200`-baud serial interface to disable automatic control and command the motor directly (`80%` speed for `'f'`/`'r'`, stopped for `'s'`), ignore unrecognized serial bytes, and automatically resume closed-loop control whenever the rotary encoder level changes.

#### Scenario: Taking manual control via serial command
- **WHEN** `'f'`, `'r'`, or `'s'` is received over serial
- **THEN** automatic closed-loop control is disabled, the motor is commanded directly with that mode at `80%` speed (or stopped for `'s'`), and a manual override message is logged

#### Scenario: Ignoring unrecognized serial input
- **WHEN** a serial character other than `'f'`, `'r'`, or `'s'` is received
- **THEN** automatic control state and motor output remain unchanged

#### Scenario: Resuming automatic control on encoder turn
- **WHEN** automatic control is disabled due to manual override and the rotary encoder reports a new level
- **THEN** the controller updates `targetADC` from the new level, re-enables automatic closed-loop control, and logs the new level and target ADC

### Requirement: Hard software travel limits
The resistance controller SHALL enforce hard software end-stops in both automatic and manual modes by immediately stopping the motor and logging a limit warning whenever the motor is running `Forward` at `adc >= 3000` or running `Reverse` at `adc <= 400`.

#### Scenario: Stopping at the maximum safe ADC limit
- **WHEN** the motor is running `Forward` (in either automatic or manual mode) and the potentiometer reading reaches `>= 3000`
- **THEN** the controller immediately stops the motor and logs a maximum-limit warning

#### Scenario: Stopping at the minimum safe ADC limit
- **WHEN** the motor is running `Reverse` (in either automatic or manual mode) and the potentiometer reading reaches `<= 400`
- **THEN** the controller immediately stops the motor and logs a minimum-limit warning

### Requirement: Periodic serial telemetry
The resistance controller SHALL emit a formatted status line over serial every `200 ms` reporting the current resistance level (`1..32`), target ADC, measured potentiometer ADC, and active motor mode character (`f`, `r`, or `s`), while continuing to run the control loop every `10 ms`.

#### Scenario: Emitting periodic telemetry
- **WHEN** `200 ms` or more has elapsed since the previous status printout
- **THEN** the controller prints the current level, target ADC, measured pot ADC, and motor mode character over serial
