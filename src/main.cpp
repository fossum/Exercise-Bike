#include <Arduino.h>
#include "potentiometer.h"

// Change this to whichever ADC pin (GPIO 0 through 6) has your wiper wire
const uint8_t POT_PIN = 4;

Potentiometer pot(POT_PIN);

void setup() {
  Serial.begin(115200);
  delay(1000);

  pot.begin();

  Serial.println("\n--- Echelon Actuator Potentiometer Test ---");
}

void loop() {
  PotentiometerReading reading = pot.read();

  // Print out the current values
  Serial.printf("ADC Raw: %4d / 4095  |  Voltage: %4d mV (%.2f V)  |  Position: %5.1f%%\n",
                reading.getRaw(),
                reading.getMilliVolts(),
                reading.getVolts(),
                reading.getPercent());

  delay(200);
}
