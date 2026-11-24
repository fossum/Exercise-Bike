# Spec Delta

## MODIFIED Requirements

### Requirement: Startup position synchronization
The resistance controller SHALL read the current physical potentiometer position at startup, clamp it to the safe operating window (`400..1920` raw ADC), map it linearly to a discrete resistance level (`1..32`), synchronize the encoder level to that initial level, initialize `targetADC` to the clamped physical ADC position (so integer `map()` quantization does not cause unintended motor movement on boot before the encoder is turned), and clear the initial encoder change event.

#### Scenario: Booting with potentiometer mid-travel
- **WHEN** the controller boots while the potentiometer reads `1700` raw ADC
- **THEN** the controller sets the initial resistance level to the corresponding mapped value in `1..32`, sets `targetADC` to `1700` (keeping the motor stationary at boot), clears the pending change flag, and logs the startup banner with initial level, pot ADC, and target ADC

#### Scenario: Booting with potentiometer outside safe window
- **WHEN** the controller boots while the potentiometer reads below `400` or above `1920` raw ADC
- **THEN** the startup mapping clamps the reading to `400` (level `1`) or `1920` (level `32`) respectively

### Requirement: Closed-loop level-to-position control
The resistance controller SHALL map resistance levels `1..32` linearly to target ADC positions `400..1920` and evaluate position error (`targetADC - adc`) on a `10 ms` control loop when automatic control is active, driving the motor at `80%` speed toward the target and stopping within a `+/- 25` ADC deadband.

#### Scenario: Driving forward toward a higher resistance level
- **WHEN** automatic control is active, `targetADC - adc > 25`, and `adc < 1920`
- **THEN** the controller drives the motor `Forward` at `80%` speed to increase the potentiometer ADC reading

#### Scenario: Driving reverse toward a lower resistance level
- **WHEN** automatic control is active, `targetADC - adc < -25`, and `adc > 400`
- **THEN** the controller drives the motor `Reverse` at `80%` speed to decrease the potentiometer ADC reading

#### Scenario: Stopping inside the deadband window
- **WHEN** automatic control is active and `|targetADC - adc| <= 25`
- **THEN** the controller stops the motor to prevent oscillation and hunting

### Requirement: Hard software travel limits
The resistance controller SHALL enforce hard software end-stops in both automatic and manual modes by immediately stopping the motor and logging a limit warning whenever the motor is running `Forward` at `adc >= 1920` or running `Reverse` at `adc <= 400`.

#### Scenario: Stopping at the maximum safe ADC limit
- **WHEN** the motor is running `Forward` (in either automatic or manual mode) and the potentiometer reading reaches `>= 1920`
- **THEN** the controller immediately stops the motor and logs a maximum-limit warning

#### Scenario: Stopping at the minimum safe ADC limit
- **WHEN** the motor is running `Reverse` (in either automatic or manual mode) and the potentiometer reading reaches `<= 400`
- **THEN** the controller immediately stops the motor and logs a minimum-limit warning
