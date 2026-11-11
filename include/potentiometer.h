#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include <Arduino.h>

class PotentiometerReading {
public:
  PotentiometerReading(int raw, int millivolts, int minRaw = 0, int maxRaw = 4095);

  int getRaw() const;
  int getMilliVolts() const;
  float getVolts() const;
  float getNormalized() const;
  float getPercent() const;

private:
  int raw_;
  int millivolts_;
  int minRaw_;
  int maxRaw_;
};

class Potentiometer {
public:
  explicit Potentiometer(uint8_t pin,
                         int samples = 16,
                         uint32_t sampleDelayMs = 2,
                         int minRaw = 0,
                         int maxRaw = 4095,
                         uint8_t resolutionBits = 12);

  void begin();
  PotentiometerReading read() const;

  void setCalibration(int minRaw, int maxRaw);
  int getMinRaw() const;
  int getMaxRaw() const;
  int getMaxAdcValue() const;

private:
  uint8_t pin_;
  int samples_;
  uint32_t sampleDelayMs_;
  int minRaw_;
  int maxRaw_;
  uint8_t resolutionBits_;
};

#endif // POTENTIOMETER_H
