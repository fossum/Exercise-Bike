# Device and Connector Pinouts

## Bike Connectors

Bike connectors are always described from the left side to the right side when looking at
the pin-socket with the notches facing you.

### 9-Pin Connector

| Pin | Signal | Board Pin | Description |
| --- | --- | --- | --- |
| 1 | Motor Positive | Motor A+ (on H-bridge) | The positive driver for the resistance motor. |
| 2 | Motor Negative | Motor A- (on H-bridge) | The negative driver for the resistance motor. |
| 3 | Potentiometer Ground | Ground | The ground pin for the potentiometer. |
| 4 | Potentiometer Signal | ADC Pin | The signal return from the potentiometer. |
| 5 | Potentiometer Vcc | 3.3V rail | The power source for the potentiometer. |
| 6 | Cadence Signal? | GPIO w/interrupt | Reed switch pulse input w/interrupt. |
| 7 | Cadence Ground? | Ground | The cadence sensor's ground return. |
| 8 | +12V DC | 12V rail | The supply from the bike's 12 volt power brick. |
| 9 | 12V GND | Ground | The supply return from the bike's power brick. |

### 3-Pin Encoder Connector

| Pin | Signal | Board Pin | Description |
| --- | --- | --- | --- |
| 1 | Signal A | GPIO w/interrupt | Quadrature Phase A (`CLK`) GPIO w/interrupt. |
| 2 | Signal B | GPIO w/interrupt | Quadrature Phase B (`DT`) GPIO w/interrupt. |
| 3 | Ground | Ground | The sensor's ground return. |

---

## Controller PCB Connectors (`J1`–`J2`)

Connector pinouts for the carrier PCB (`pcb/echelon_ex5s_controller.kicad_sch` / `pcb/echelon_ex5s_controller.kicad_pcb`), using vertical 2.54 mm (0.1 in) pitch headers that match the bike's 9-pin and 3-pin harnesses 1:1.

### `J1` — `Bike_Harness_9P` (`1x09`, `2.54 mm` Pitch)

| Pin | Symbol Pin | PCB Net | Connected To | Description |
| --- | --- | --- | --- | --- |
| 1 | `MOT_RED` | `MOT_RED` | `U3` Pin 7 (`MOTA_1`) | Motor Positive (H-bridge channel A output 1). |
| 2 | `MOT_BLK` | `MOT_BLK` | `U3` Pin 8 (`MOTA_2`) | Motor Negative (H-bridge channel A output 2). |
| 3 | `POT_GND` | `GND` | Common `GND` | Potentiometer ground return. |
| 4 | `POT_WIPER` | `POT_WIPER` | `U2` `GPIO 2` (`POT_PIN`), `C3` (`100 nF`) | Potentiometer ADC wiper signal. |
| 5 | `POT_3V3` | `+3V3` | `U2` `3V3` rail | Potentiometer excitation (`+3V3`, never `+5V`). |
| 6 | `CADENCE` | `CADENCE` | `U2` `GPIO 4` (`CADENCE_PIN`) | Cadence reed-switch pulse input. |
| 7 | `CADENCE_GND` | `GND` | Common `GND` | Cadence sensor ground return. |
| 8 | `VIN_12V` | `+12V` | `U1` Pin 1 (`VIN_12V`) | `+12V` DC power supply from bike power brick. |
| 9 | `GND_12V` | `GND` | Common `GND` | `12V` DC power supply return ground. |

### `J2` — `Encoder_3P` (`1x03`, `2.54 mm` Pitch)

| Pin | Symbol Pin | PCB Net | Connected To | Description |
| --- | --- | --- | --- | --- |
| 1 | `ENC_A` | `ENC_A` | `U2` `GPIO 6` (`ENC_PIN_A`) | Quadrature Phase A (`CLK`). |
| 2 | `ENC_B` | `ENC_B` | `U2` `GPIO 5` (`ENC_PIN_B`) | Quadrature Phase B (`DT`). |
| 3 | `GND` | `GND` | Common `GND` | Encoder ground return. |

---

## Target Board

- **Board:** `esp32-c6-devkitc-1` (ESP32-C6-DevKitC-1)

> **Pin Reservation Note:** Avoid `GPIO 8` on the ESP32-C6-DevKitC-1, as it is used by the onboard RGB LED. GPIO 12 and GPIO 13: Used by the ESP32-C6's native USB Serial/JTAG interface.

---

## ESP32-C6 GPIO Assignment Summary

| GPIO | Code Constant | Device / Subsystem | Signal / Function | Mode | Notes |
| --- | --- | --- | --- | --- | --- |
| `GPIO 2` | `POT_PIN` | Resistance Potentiometer | Wiper (ADC1) | `INPUT` (12-bit ADC) | 3.3V max (power pot from 3V3, not 5V); safe range 400–1920 raw ADC |
| `GPIO 4` | `CADENCE_PIN` | Cadence Sensor *(planned)* | Reed Switch Pulse | `INPUT_PULLUP` | Defined in `include/pins.h`; hardware interrupt (`FALLING`) to GND; debounce in code to compute RPM |
| `GPIO 5` | `ENC_PIN_B` | Rotary Encoder | Phase B (`DT`) | `INPUT_PULLUP` | Hardware interrupt (`CHANGE`) |
| `GPIO 6` | `ENC_PIN_A` | Rotary Encoder | Phase A (`CLK`) | `INPUT_PULLUP` | Hardware interrupt (`CHANGE`) |
| `GPIO 7` | `PIN_STBY` | Resistance Motor Driver | Standby | `OUTPUT` | Drive `HIGH` (3.3V) to enable H-bridge; `LOW` (0V) to standby when idle |
| `GPIO 19` | `PIN_AIN2` | Resistance Motor Driver | `AIN2` | `OUTPUT` | Active `HIGH` for Reverse (decreases Pot ADC) |
| `GPIO 20` | `PIN_AIN1` | Resistance Motor Driver | `AIN1` | `OUTPUT` | Active `HIGH` for Forward (increases Pot ADC) |
| `GPIO 21` | `PIN_PWM` | Resistance Motor Driver | `PWM` | `OUTPUT` | 8-bit PWM speed control (currently set to 80% speed) |

---

## Device Details

### 1. Resistance Potentiometer (`Potentiometer`)

| Signal | ESP32-C6 Pin | Configuration |
| --- | --- | --- |
| Wiper (Analog Output) | `GPIO 2` (`POT_PIN`) | `INPUT`, 12-bit ADC (`0`–`4095`), 16-sample average, `0 ms` sample delay |

---

### 2. Resistance Motor Driver (`Motor`)

| Signal | ESP32-C6 Pin | Configuration |
| --- | --- | --- |
| `AIN1` | `GPIO 20` (`PIN_AIN1`) | `OUTPUT` |
| `AIN2` | `GPIO 19` (`PIN_AIN2`) | `OUTPUT` |
| `STBY` *(optional)* | `GPIO 7` (`PIN_STBY`) | `OUTPUT` (`HIGH` when running, `LOW` when stopped/idle) |
| `PWM` *(optional)* | `GPIO 21` (`PIN_PWM`) | `OUTPUT` (8-bit resolution, 80% speed) |

**Direction Logic:**

`PIN_AIN1` and `PIN_AIN2` are assigned so that `Forward` increases the potentiometer ADC reading and `Reverse` decreases it:

| Mode | Serial Command | `PIN_AIN1` (`GPIO 20`) | `PIN_AIN2` (`GPIO 19`) | `PIN_PWM` (`GPIO 21`) | `PIN_STBY` (`GPIO 7`) | Effect |
| --- | --- | --- | --- | --- | --- | --- |
| `Forward` | `'f'` | `HIGH` | `LOW` | `80%` (`204`) | `HIGH` | Increases Pot ADC (higher resistance) |
| `Reverse` | `'r'` | `LOW` | `HIGH` | `80%` (`204`) | `HIGH` | Decreases Pot ADC (lower resistance) |
| `Stopped` | `'s'` | `LOW` | `LOW` | `0%` (`0`) | `LOW` | Motor stopped (H-bridge in standby) |

---

### 3. Rotary Encoder (`RotaryEncoder`)

| Signal | ESP32-C6 Pin | Configuration |
| --- | --- | --- |
| Phase A (`CLK`) | `GPIO 6` (`ENC_PIN_A`) | `INPUT_PULLUP`, `CHANGE` interrupt |
| Phase B (`DT`) | `GPIO 5` (`ENC_PIN_B`) | `INPUT_PULLUP`, `CHANGE` interrupt |
| Push Button (`SW`) | Not wired (`-1`, `ENC_PIN_SW`) | Optional `INPUT_PULLUP` (active-low, 200 ms debounce) |

---

### 4. Cadence Sensor *(Planned)*

| Signal | ESP32-C6 Pin | Configuration |
| --- | --- | --- |
| Reed Switch Pulse | `GPIO 4` (`CADENCE_PIN`) | `INPUT_PULLUP`, `FALLING` interrupt (defined in `include/pins.h`; firmware RPM tracking planned) |
