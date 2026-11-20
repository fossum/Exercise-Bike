# Device and Connector Pinouts

## Bike Connectors

Bike connectors are always described from the left side to the right side when looking at
the pin-socket with the notches facing you.

### 9-Pin Connector

| Signal | Board Pin | Description |
| --- | --- | --- |
| Motor Positive | Motor A+ (on H-bridge) | The positive driver for the resistance motor. |
| Motor Negative | Motor A- (on H-bridge) | The negative driver for the resistance motor. |
| Potentiometer Ground | Ground | The ground pin for the potentiometer. |
| Potentiometer Signal | ADC Pin | The signal return from the potentiometer. |
| Potentiometer Vcc | 3.3V rail | The power source for the potentiometer. |
| Cadence Signal? | w/interrupt | |
| Cadence Ground? | Ground | |
| +12V DC | 12V rail | The supply from the bike's 12 volt power brick. |
| 12V GND | Ground | The supply return from the bike's power brick. |

### 3-Pin Encoder Connector

| Signal | Board Pin | Description |
| --- | --- | --- |
| Signal A | GPIO w/interrupt | |
| Signal B | GPIO w/interrupt | |
| Ground | Ground | The sensor's ground return. |

## Target Board

- **Board:** `esp32-c6-devkitc-1` (ESP32-C6-DevKitC-1)

> **Pin Reservation Note:** Avoid `GPIO 8` on the ESP32-C6-DevKitC-1, as it is used by the onboard RGB LED. GPIO 12 and GPIO 13: Used by the ESP32-C6's native USB Serial/JTAG interface.

---

## ESP32-C6 GPIO Assignment Summary

| GPIO | Code Constant | Device / Subsystem | Signal / Function | Mode | Notes |
| --- | --- | --- | --- | --- | --- |
| `GPIO 2` | `POT_PIN` | Resistance Potentiometer | Wiper (ADC1) | `INPUT` (12-bit ADC) | 3.3V max (power pot from 3V3, not 5V); safe range 400–3000 raw ADC |
| `GPIO 5` | `ENC_PIN_B` | Rotary Encoder | Phase B (`DT`) | `INPUT_PULLUP` | Hardware interrupt (`CHANGE`) |
| `GPIO 6` | `ENC_PIN_A` | Rotary Encoder | Phase A (`CLK`) | `INPUT_PULLUP` | Hardware interrupt (`CHANGE`) |
| `GPIO 18` | `PIN_AIN2` | Resistance Motor Driver | `AIN2` | `OUTPUT` | Active `HIGH` for Reverse (decreases Pot ADC) |
| `GPIO 19` | `PIN_AIN1` | Resistance Motor Driver | `AIN1` | `OUTPUT` | Active `HIGH` for Forward (increases Pot ADC) |
| `GPIO 20` | `PIN_PWM` | Resistance Motor Driver | `PWM` | `OUTPUT` | 8-bit PWM speed control (currently set to 80% speed) |
| `GPIO 4` | `CADENCE_PIN` | Cadence Sensor | Reed Switch Pulse | INPUT_PULLUP | Hardware interrupt (FALLING) to GND; debounce in code to compute RPM. |
| `GPIO 7` | `PIN_STBY` | Resistance Motor Driver | Standby | OUTPUT | Drive HIGH (3.3V) to enable H-bridge; LOW (0V) to standby when idle. |

---

## Device Details

### 1. Resistance Potentiometer (`Potentiometer`)

| Signal | ESP32-C6 Pin | Configuration |
| --- | --- | --- |
| Wiper (Analog Output) | `GPIO 4` (`POT_PIN`) | `INPUT`, 12-bit ADC (`0`–`4095`), 16-sample average, `0 ms` sample delay |

---

### 2. Resistance Motor Driver (`Motor`)

| Signal | ESP32-C6 Pin | Configuration |
| --- | --- | --- |
| `AIN1` | `GPIO 19` (`PIN_AIN1`) | `OUTPUT` |
| `AIN2` | `GPIO 18` (`PIN_AIN2`) | `OUTPUT` |
| `STBY` *(optional)* | `GPIO 7` (`PIN_STBY`) | `OUTPUT` (`HIGH` when running, `LOW` when stopped/idle) |
| `PWM` *(optional)* | `GPIO 20` (`PIN_PWM`) | `OUTPUT` (8-bit resolution, 80% speed) |

**Direction Logic:**

`PIN_AIN1` and `PIN_AIN2` are assigned so that `Forward` increases the potentiometer ADC reading and `Reverse` decreases it:

| Mode | Serial Command | `PIN_AIN1` (`GPIO 19`) | `PIN_AIN2` (`GPIO 18`) | `PIN_STBY` (`GPIO 7`) | Effect |
| --- | --- | --- | --- | --- | --- |
| `Forward` | `'f'` | `HIGH` | `LOW` | `HIGH` | Increases Pot ADC (higher resistance) |
| `Reverse` | `'r'` | `LOW` | `HIGH` | `HIGH` | Decreases Pot ADC (lower resistance) |
| `Stopped` | `'s'` | `LOW` | `LOW` | `LOW` | Motor stopped (H-bridge in standby) |

---

### 3. Rotary Encoder (`RotaryEncoder`)

| Signal | ESP32-C6 Pin | Configuration |
| --- | --- | --- |
| Phase A (`CLK`) | `GPIO 6` (`ENC_PIN_A`) | `INPUT_PULLUP`, `CHANGE` interrupt |
| Phase B (`DT`) | `GPIO 5` (`ENC_PIN_B`) | `INPUT_PULLUP`, `CHANGE` interrupt |
| Push Button (`SW`) | Not wired (`-1`, `ENC_PIN_SW`) | Optional `INPUT_PULLUP` (active-low, 200 ms debounce) |
