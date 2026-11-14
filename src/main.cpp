#ifdef ARDUINO
#include <Arduino.h>
#include "potentiometer.h"
#include "motor.h"
#include "rotary_encoder.h"

#ifndef PIO_UNIT_TESTING

const uint8_t POT_PIN  = 4;  // Working potentiometer ADC pin
const uint8_t PIN_AIN1 = 19; // Swapped so Forward ('f') increases Pot ADC
const uint8_t PIN_AIN2 = 18; // and Reverse ('r') decreases Pot ADC

// Safe GPIO pins on ESP32-C6 (avoid GPIO 8 which is onboard RGB LED)
const uint8_t ENC_PIN_A  = 6;
const uint8_t ENC_PIN_B  = 5;
const int ENC_PIN_SW = -1; // Set to a GPIO pin if using the push button

// Safe software end-stops so the motor never binds at the ends of the pot
const int MIN_SAFE_ADC = 400;
const int MAX_SAFE_ADC = 3000;
const int ADC_DEADBAND = 25; // Tolerance window to prevent motor hunting/oscillation

Potentiometer pot(POT_PIN, 16, 0, MIN_SAFE_ADC, MAX_SAFE_ADC, 12);
Motor motor(PIN_AIN1, PIN_AIN2);
RotaryEncoder encoder(ENC_PIN_A, ENC_PIN_B, ENC_PIN_SW, 1, 32);

int targetADC = MIN_SAFE_ADC;
bool autoControlEnabled = true;
uint32_t lastPrintMs = 0;

int levelToTargetADC(int level) {
    return map(level, 1, 32, MIN_SAFE_ADC, MAX_SAFE_ADC);
}

void setup() {
    Serial.begin(115200);

    pot.begin();
    motor.begin();
    encoder.begin();

    // Sync initial encoder level to the current physical potentiometer position
    int initialADC = pot.read().getRaw();
    int clampedADC = constrain(initialADC, MIN_SAFE_ADC, MAX_SAFE_ADC);
    int initialLevel = map(clampedADC, MIN_SAFE_ADC, MAX_SAFE_ADC, 1, 32);
    encoder.setLevel(initialLevel);
    encoder.hasChanged(); // Clear initial change flag
    targetADC = levelToTargetADC(initialLevel);

    Serial.println("\n--- Exercise Bike Resistance Controller ---");
    Serial.printf("Initial Level: %2d / 32  |  Pot ADC: %4d  |  Target ADC: %4d\n",
                  initialLevel, initialADC, targetADC);
    Serial.println("Turn encoder to set level (1-32), or send 'f', 'r', 's' for manual override.");
}

void loop() {
    PotentiometerReading reading = pot.read();
    int adc = reading.getRaw();

    // 1. Check for Rotary Encoder changes
    if (encoder.hasChanged()) {
        int level = encoder.getLevel();
        targetADC = levelToTargetADC(level);
        autoControlEnabled = true;

        Serial.printf("Resistance Level: %2d / 32  |  Target ADC: %4d\n", level, targetADC);
    }

    // 2. Check for Serial commands ('f', 'r', 's') for manual override
    if (Serial.available() > 0) {
        char c = static_cast<char>(Serial.read());
        if (c == 'f' || c == 'r' || c == 's') {
            autoControlEnabled = false;
            motor.setModeChar(c);
            Serial.printf("Manual override: '%c'\n", c);
        }
    }

    // 3. Closed-loop position control when in automatic (encoder) mode
    if (autoControlEnabled) {
        int error = targetADC - adc;
        if (abs(error) <= ADC_DEADBAND) {
            motor.stop();
        } else if (error > 0 && adc < MAX_SAFE_ADC) {
            motor.forward();
        } else if (error < 0 && adc > MIN_SAFE_ADC) {
            motor.reverse();
        } else {
            motor.stop();
        }
    }

    // 4. Hard safety cutoff near the ends of the potentiometer track
    if (motor.getDirection() == MotorDirection::Forward && adc >= MAX_SAFE_ADC) {
        Serial.println("Reached MAX safe limit! Stopping.");
        motor.stop();
    } else if (motor.getDirection() == MotorDirection::Reverse && adc <= MIN_SAFE_ADC) {
        Serial.println("Reached MIN safe limit! Stopping.");
        motor.stop();
    }

    // 5. Periodic status print (every 200ms) while running a fast 10ms control loop
    uint32_t now = millis();
    if (now - lastPrintMs >= 200) {
        lastPrintMs = now;
        Serial.printf("Level: %2d/32  |  Target ADC: %4d  |  Pot ADC: %4d  |  Mode: %c\n",
                      encoder.getLevel(), targetADC, adc, motor.getModeChar());
    }

    delay(10);
}

#endif // PIO_UNIT_TESTING
#else // !ARDUINO

#ifndef PIO_UNIT_TESTING
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return 0;
}
#endif // PIO_UNIT_TESTING

#endif // ARDUINO
