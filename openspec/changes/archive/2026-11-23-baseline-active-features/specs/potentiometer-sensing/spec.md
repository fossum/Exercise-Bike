# Spec Delta

## Purpose

Provides calibrated, noise-averaged analog position sensing from the resistance potentiometer, reporting raw ADC counts, voltage, and bounded normalized position without coupling to specific board pins.

## ADDED Requirements

### Requirement: Multi-sample averaged ADC reading
The potentiometer sensing capability SHALL average a configurable number of consecutive raw ADC and millivolt readings per measurement, enforcing a minimum of one sample when a non-positive sample count is configured, and optionally waiting a configured delay interval after each sample.

#### Scenario: Averaging multiple samples with sample delay
- **WHEN** a reading is taken with a sample count of 4, a sample delay of 3 ms, raw samples `[1000, 1010, 1020, 1030]`, and millivolt samples `[800, 810, 820, 830]`
- **THEN** the resulting reading reports an averaged raw value of `1015`, an averaged millivolt value of `815`, and invokes the delay interval 4 times for a total of 12 ms

#### Scenario: Zero sample delay skips waiting
- **WHEN** a reading is taken with a configured sample delay of `0` ms
- **THEN** the averaged reading is computed without invoking any inter-sample delay

#### Scenario: Non-positive sample count defaults to a single sample
- **WHEN** the sensor is configured with a sample count of `0` or less
- **THEN** the sensor takes exactly 1 sample per reading without dividing by zero

### Requirement: Voltage and ADC range reporting
The potentiometer sensing capability SHALL expose measured voltage in both millivolts and volts (`millivolts / 1000`) and report the maximum raw ADC value corresponding to its configured bit resolution (`2^resolutionBits - 1`).

#### Scenario: Reporting voltage and 12-bit maximum ADC range
- **WHEN** a reading has an averaged millivolt measurement of `1650` mV on a sensor configured for 12-bit resolution
- **THEN** the reading reports `1650` mV and `1.65` V, and the sensor reports a maximum ADC value of `4095`

### Requirement: Calibrated and clamped position normalization
The potentiometer sensing capability SHALL normalize raw ADC readings linearly across configurable minimum and maximum calibration bounds into a normalized ratio in `[0.0, 1.0]` and a percentage in `[0.0, 100.0]`, clamping values outside the bounds and returning `0.0` whenever the calibration range is invalid (`maxRaw <= minRaw`).

#### Scenario: Midpoint normalization within custom calibration bounds
- **WHEN** calibration bounds are set to `minRaw = 500` and `maxRaw = 3500` and the raw reading is `2000`
- **THEN** the normalized position is `0.5` and the percentage is `50.0%`

#### Scenario: Clamping readings outside calibration bounds
- **WHEN** calibration bounds are `500..3500` and raw readings of `200` and `3900` are evaluated
- **THEN** `200` clamps to `0.0` (`0.0%`) and `3900` clamps to `1.0` (`100.0%`)

#### Scenario: Updating calibration bounds at runtime
- **WHEN** calibration bounds are updated to `1000..2000` and a raw reading of `1500` is taken
- **THEN** the sensor reports the updated bounds and normalizes the reading to `0.5` (`50.0%`)

#### Scenario: Invalid calibration bounds fail safely
- **WHEN** calibration bounds have `maxRaw <= minRaw` (such as `minRaw = 3000`, `maxRaw = 1000`)
- **THEN** the normalized position safely returns `0.0` and the percentage returns `0.0%`

### Requirement: Injectable hardware readers for host testing
The potentiometer sensing capability SHALL allow callers to supply custom raw-ADC reader, millivolt reader, and delay callbacks so measurement logic can be exercised independently of physical ADC hardware.

#### Scenario: Custom readers supply deterministic measurements
- **WHEN** custom raw reader, millivolt reader, and delay callbacks are installed and a reading is requested
- **THEN** the sensor invokes the injected callbacks rather than physical hardware ADC peripherals
