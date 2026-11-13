#ifdef ARDUINO
#include <Arduino.h>
#include "potentiometer.h"
#include "motor.h"

#ifndef PIO_UNIT_TESTING

const uint8_t POT_PIN  = 4;  // Working potentiometer ADC pin
const uint8_t PIN_AIN1 = 19; // Swapped so Forward ('f') increases Pot ADC
const uint8_t PIN_AIN2 = 18; // and Reverse ('r') decreases Pot ADC

// Safe software end-stops so the motor never binds at the ends of the pot
const int MIN_SAFE_ADC = 400;
const int MAX_SAFE_ADC = 3000;

Potentiometer pot(POT_PIN, 16, 0, MIN_SAFE_ADC, MAX_SAFE_ADC, 12);
Motor motor(PIN_AIN1, PIN_AIN2);

void setup() {
  Serial.begin(115200);

  pot.begin();
  motor.begin();

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
