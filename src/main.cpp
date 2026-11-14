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

Potentiometer pot(POT_PIN, 16, 0, MIN_SAFE_ADC, MAX_SAFE_ADC, 12);
Motor motor(PIN_AIN1, PIN_AIN2);
RotaryEncoder encoder(ENC_PIN_A, ENC_PIN_B, ENC_PIN_SW, 1, 32);

void setup() {
    Serial.begin(115200);

    pot.begin();
    motor.begin();
    encoder.begin();
    Serial.println("Encoder initialized. Level: 1");

    Serial.println("\n--- Motor + Potentiometer Closed-Loop Test ---");
    Serial.println("Send 'f' (Forward), 'r' (Reverse), or 's' (Stop)");
}

void loop() {
    PotentiometerReading reading = pot.read();
    int adc = reading.getRaw();

    // Check for Serial commands ('f', 'r', 's')
    if (Serial.available() > 0) {
        char c = static_cast<char>(Serial.read());
        if (c == 'f' || c == 'r' || c == 's') {
            motor.setModeChar(c);
        }
    }

    // Automatic safety cutoff near the ends of the potentiometer track
    if (motor.getDirection() == MotorDirection::Forward && adc >= MAX_SAFE_ADC) {
        Serial.println("Reached MAX safe limit! Stopping.");
        motor.stop();
    } else if (motor.getDirection() == MotorDirection::Reverse && adc <= MIN_SAFE_ADC) {
        Serial.println("Reached MIN safe limit! Stopping.");
        motor.stop();
    }

    if (encoder.hasChanged()) {
        int level = encoder.getLevel();

        // Map level 1-32 to your potentiometer's safe ADC span
        int targetADC = map(level, 1, 32, MIN_SAFE_ADC, MAX_SAFE_ADC);

        Serial.printf("Resistance Level: %2d / 32  |  Target ADC: %4d\n", level, targetADC);

        // Pass targetADC to your Motor controller class here:
        // motor.setTargetPosition(targetADC);
    }

    Serial.printf("Mode: %c  |  Pot ADC: %4d\n", motor.getModeChar(), adc);
    delay(100);
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
