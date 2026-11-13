#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <stdint.h>
#ifndef INPUT
#define INPUT 0x01
#endif
#endif

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
  using AdcReadFn = int (*)(uint8_t pin);
  using DelayFn = void (*)(uint32_t ms);

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

  void setHardwareReaders(AdcReadFn rawReader, AdcReadFn mvReader, DelayFn delayFn = nullptr);

private:
  uint8_t pin_;
  int samples_;
  uint32_t sampleDelayMs_;
  int minRaw_;
  int maxRaw_;
  uint8_t resolutionBits_;
  AdcReadFn rawReader_;
  AdcReadFn mvReader_;
  DelayFn delayFn_;
};

#endif // POTENTIOMETER_H
