# Spec Delta

## Purpose

Defines the electrical architecture, power-rail regulation, decoupling, connector interfaces, H-bridge motor driver wiring, and ESP32-C6-DevKitC-1 GPIO assignments for the Exercise-Bike controller carrier PCB.

## ADDED Requirements

### Requirement: Power rail regulation and distribution
The controller PCB SHALL accept a `+12V` DC input, step it down to a regulated `+5V` rail via a buck converter module (`Buck_12V_to_5V`) to power the `ESP32-C6-DevKitC-1` and the `Adafruit_TB6612` motor supply (`VM`), and use the ESP32-C6's `+3V3` rail for logic (`VCC`) and potentiometer excitation (`+3V3`, never `+5V`).

#### Scenario: Powering logic and motor rails from 12V bike supply
- **WHEN** `+12V` DC and `GND` are supplied at `J1` (`Bike_Harness_9P`, Pins 8–9)
- **THEN** the buck converter supplies `+5V` to `ESP32-C6-DevKitC-1` and `TB6612` `VM`, and `+3V3` powers `TB6612` logic `VCC` and the potentiometer excitation pin

### Requirement: Motor power decoupling and ADC input filtering
The controller PCB SHALL include a `220 uF` bulk electrolytic capacitor (`C1`) and a `100 nF` ceramic decoupling capacitor (`C2`) across `+5V`/`VM` and `GND` near the motor driver, and a `100 nF` ceramic filter capacitor (`C3`) between `POT_WIPER` and `GND` near the ADC input.

#### Scenario: Filtering motor transients and potentiometer noise
- **WHEN** the resistance motor switches or modulates PWM on `VM`
- **THEN** `C1` and `C2` decouple the `+5V`/`VM` rail locally at `U3`, and `C3` shunts high-frequency noise on `POT_WIPER` to `GND`

### Requirement: Bike harness and peripheral connectors
The controller PCB SHALL provide 2.54 mm (0.1 in) pitch vertical headers matching the bike harnesses 1:1: `J1` (`1x09`, `2.54 mm`) for motor (`MOT_RED` Pin 1, `MOT_BLK` Pin 2), potentiometer (`GND` Pin 3, `POT_WIPER` Pin 4, `+3V3` Pin 5), cadence (`CADENCE` Pin 6, `GND` Pin 7), and `+12V` power input (`+12V` Pin 8, `GND` Pin 9); and `J2` (`1x03`, `2.54 mm`) for the rotary encoder (`ENC_A` Pin 1, `ENC_B` Pin 2, `GND` Pin 3).

#### Scenario: Connecting bike resistance and encoder harnesses
- **WHEN** the bike's 9-pin motor/potentiometer/cadence/12V harness and 3-pin rotary encoder harness are plugged into `J1` and `J2`
- **THEN** all motor, potentiometer, cadence, encoder, and power signals map 1:1 to their corresponding PCB nets without crossing voltage domains

### Requirement: ESP32-C6-DevKitC-1 GPIO assignment
The controller PCB SHALL route `ESP32-C6-DevKitC-1` (`U2`) signals to match the firmware pin contract: `GPIO 2` (`POT_WIPER`), `GPIO 4` (`CADENCE`), `GPIO 5` (`ENC_B`), `GPIO 6` (`ENC_A`), `GPIO 7` (`STBY`), `GPIO 20` (`AIN1`), `GPIO 19` (`AIN2`), and `GPIO 21` (`PWMA`), and SHALL NOT assign `GPIO 8` (onboard RGB LED) or `GPIO 12`/`GPIO 13` (native USB Serial/JTAG).

#### Scenario: Verifying schematic GPIO nets against firmware constants
- **WHEN** the `U2` (`ESP32-C6-DevKitC-1`) symbol connections in the schematic are inspected
- **THEN** every signal net matches `include/pins.h` (`POT_PIN=2`, `CADENCE_PIN=4`, `ENC_PIN_B=5`, `ENC_PIN_A=6`, `PIN_STBY=7`, `PIN_AIN2=19`, `PIN_AIN1=20`, `PIN_PWM=21`) and leaves `GPIO 8`, `GPIO 12`, and `GPIO 13` unconnected

### Requirement: TB6612 H-bridge driver integration
The controller PCB SHALL connect `Adafruit_TB6612` (`U3`) inputs `AIN1`, `AIN2`, `PWMA`, and `STBY` to the corresponding `ESP32-C6-DevKitC-1` GPIO nets and drive `MOT_RED` and `MOT_BLK` on `J1` (Pins 1–2) from motor channel A outputs such that `AIN1=HIGH, AIN2=LOW` increases `POT_WIPER` voltage.

#### Scenario: Enabling and driving the resistance motor via TB6612
- **WHEN** `STBY` (`GPIO 7`) is driven `HIGH`, `PWMA` (`GPIO 21`) is modulated, and `AIN1` (`GPIO 20`) / `AIN2` (`GPIO 19`) are driven `HIGH`/`LOW`
- **THEN** `U3` exits standby and drives `MOT_RED`/`MOT_BLK` to increase the potentiometer ADC reading
