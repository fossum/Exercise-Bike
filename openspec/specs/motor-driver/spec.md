# motor-driver Specification

## Purpose

Controls the bidirectional H-bridge resistance motor with optional PWM speed modulation, optional standby power management, and single-character mode commands, independent of board-specific pin assignments.

## Requirements

### Requirement: H-bridge directional control and safe initialization
The motor driver capability SHALL manage three mutually exclusive operating directions (`Forward`, `Reverse`, `Stopped`), initialize all configured control outputs in the `Stopped` state with `0%` speed, and drive the two H-bridge direction outputs accordingly (`Forward`: `AIN1=HIGH, AIN2=LOW`; `Reverse`: `AIN1=LOW, AIN2=HIGH`; `Stopped`: `AIN1=LOW, AIN2=LOW`).

#### Scenario: Initializing the motor driver
- **WHEN** the motor driver is initialized
- **THEN** all configured direction, optional PWM, and optional standby channels are configured as outputs and the motor starts in the `Stopped` state with `AIN1=LOW`, `AIN2=LOW`, and `0%` speed

#### Scenario: Transitioning between Forward, Reverse, and Stopped
- **WHEN** the motor is commanded `Forward`, then `Reverse`, then `Stopped`
- **THEN** `Forward` sets `AIN1=HIGH, AIN2=LOW` and reports running; `Reverse` sets `AIN1=LOW, AIN2=HIGH` and reports running; and `Stopped` sets `AIN1=LOW, AIN2=LOW`, `0%` speed, and reports not running

### Requirement: Optional PWM speed scaling and clamping
The motor driver capability SHALL support an optional PWM output channel with configurable bit resolution (defaulting to 8-bit when unspecified or zero), clamping requested speed percentages to `[0.0, 100.0]`, scaling active speed to the nearest integer PWM count (`0..2^bits - 1`), and transitioning to `Stopped` when speed is `<= 0%`.

#### Scenario: Scaling and rounding PWM duty cycle
- **WHEN** an 8-bit PWM channel is configured and the motor is commanded `Forward` at `50.0%` speed
- **THEN** the driver reports `50.0%` (`0.5` normalized) and writes a PWM value of `128` (rounded from `0.5 * 255`)

#### Scenario: Clamping overspeed requests to 100 percent
- **WHEN** speed is updated to `150.0%` while the motor is running with an 8-bit PWM channel
- **THEN** speed clamps to `100.0%` and the PWM output is set to `255`

#### Scenario: Non-positive speed stops the motor
- **WHEN** the motor is commanded `Reverse` with `-10.0%` speed or its running speed is updated to `0.0%`
- **THEN** the motor transitions to `Stopped` with `AIN1=LOW`, `AIN2=LOW`, and PWM output `0`

#### Scenario: Setting speed while stopped does not start the motor
- **WHEN** speed is updated to `60.0%` while the motor is `Stopped`
- **THEN** the motor remains `Stopped` with PWM output `0`

#### Scenario: Operating without a configured PWM channel
- **WHEN** the motor driver is configured without a PWM channel
- **THEN** direction changes succeed using digital outputs only and no analog PWM writes are performed

### Requirement: Optional H-bridge standby control and sequencing
The motor driver capability SHALL support an optional active-low standby output that is held `LOW` whenever the motor is stopped and driven `HIGH` whenever the motor is running, de-asserting standby `LOW` before updating direction/PWM on stop and asserting standby `HIGH` after updating direction/PWM on start.

#### Scenario: Entering and exiting standby around motor motion
- **WHEN** a standby channel is configured and the motor initializes, runs `Forward` at `80.0%`, then stops
- **THEN** standby is driven `LOW` on initialization, driven `HIGH` after direction and PWM outputs are applied on `Forward`, and driven `LOW` before direction and PWM outputs are cleared on `Stopped`

#### Scenario: Operating without a configured standby channel
- **WHEN** no standby channel is configured
- **THEN** the driver reports not in standby regardless of running state and performs no standby output writes

### Requirement: Single-character mode command mapping
The motor driver capability SHALL map single-character mode commands `'f'` to `Forward`, `'r'` to `Reverse`, and `'s'` to `Stopped`, expose the active state using those same characters, and ignore any unrecognized mode character without altering motor state.

#### Scenario: Applying valid mode characters
- **WHEN** mode commands `'f'` (at `75.0%`), `'r'` (at `25.0%`), and `'s'` are applied in sequence
- **THEN** the motor switches to `Forward` at `75.0%`, then `Reverse` at `25.0%`, then `Stopped`, reporting `'f'`, `'r'`, and `'s'` respectively

#### Scenario: Ignoring unrecognized mode characters
- **WHEN** an unrecognized mode character such as `'x'` is applied while the motor is running `Forward` at `75.0%`
- **THEN** the motor remains `Forward` at `75.0%` with no state change

### Requirement: Injectable hardware writers for host testing
The motor driver capability SHALL allow callers to supply custom digital-write, analog-write, and pin-mode callbacks so H-bridge state and output writes can be verified without physical hardware.

#### Scenario: Verifying outputs through injected writers
- **WHEN** custom digital-write, analog-write, and pin-mode callbacks are installed and motor commands are issued
- **THEN** the driver routes all pin configuration and output updates through the injected callbacks
