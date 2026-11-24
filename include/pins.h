#ifndef PINS_H
#define PINS_H

#include <stdint.h>

// Target Board: ESP32-C6-DevKitC-1 (esp32-c6-devkitc-1)
// Note: Avoid GPIO 8 (onboard RGB LED) and GPIO 12/13 (native USB Serial/JTAG).

// 1. Resistance Potentiometer
constexpr uint8_t POT_PIN = 2; // Working potentiometer ADC1 wiper pin

// 2. Resistance Motor Driver (H-bridge)
constexpr uint8_t PIN_AIN1 = 20; // Forward ('f') increases Pot ADC
constexpr uint8_t PIN_AIN2 = 19; // Reverse ('r') decreases Pot ADC
constexpr uint8_t PIN_PWM  = 21; // H-bridge PWM speed control pin
constexpr uint8_t PIN_STBY = 7;  // H-bridge standby pin (HIGH = enabled, LOW = standby)

// 3. Rotary Encoder
constexpr uint8_t ENC_PIN_A = 6; // Phase A (CLK)
constexpr uint8_t ENC_PIN_B = 5; // Phase B (DT)
constexpr int ENC_PIN_SW = -1;   // Push button (SW), -1 when not wired

// 4. Cadence Sensor
constexpr uint8_t CADENCE_PIN = 4; // Reed switch pulse input (active-low with pullup)

#endif // PINS_H
