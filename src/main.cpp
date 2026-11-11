#include <Arduino.h>

// Change this to whichever ADC pin (GPIO 0 through 6) has your wiper wire
const int POT_PIN = 4;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Set ADC resolution to 12-bit (0 - 4095)
  analogReadResolution(12);

  Serial.println("\n--- Echelon Actuator Potentiometer Test ---");
}

void loop() {
  // Average 16 quick readings to filter out breadboard noise
  long rawSum = 0;
  long mvSum = 0;
  const int SAMPLES = 16;

  for (int i = 0; i < SAMPLES; i++) {
    rawSum += analogRead(POT_PIN);
    mvSum  += analogReadMilliVolts(POT_PIN);
    delay(2);
  }

  int rawAvg = rawSum / SAMPLES;
  int mvAvg  = mvSum / SAMPLES;

  // Print out the current values
  Serial.printf("ADC Raw: %4d / 4095  |  Voltage: %4d mV (%.2f V)\n",
                rawAvg, mvAvg, mvAvg / 1000.0);

  delay(200);
}
