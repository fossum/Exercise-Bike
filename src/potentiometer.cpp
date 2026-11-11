#include "potentiometer.h"

PotentiometerReading::PotentiometerReading(int raw, int millivolts, int minRaw, int maxRaw)
    : raw_(raw), millivolts_(millivolts), minRaw_(minRaw), maxRaw_(maxRaw) {}

int PotentiometerReading::getRaw() const {
  return raw_;
}

int PotentiometerReading::getMilliVolts() const {
  return millivolts_;
}

float PotentiometerReading::getVolts() const {
  return millivolts_ / 1000.0f;
}

float PotentiometerReading::getNormalized() const {
  if (maxRaw_ <= minRaw_) {
    return 0.0f;
  }
  float norm = static_cast<float>(raw_ - minRaw_) / static_cast<float>(maxRaw_ - minRaw_);
  if (norm < 0.0f) {
    return 0.0f;
  }
  if (norm > 1.0f) {
    return 1.0f;
  }
  return norm;
}

float PotentiometerReading::getPercent() const {
  return getNormalized() * 100.0f;
}

Potentiometer::Potentiometer(uint8_t pin,
                             int samples,
                             uint32_t sampleDelayMs,
                             int minRaw,
                             int maxRaw,
                             uint8_t resolutionBits)
    : pin_(pin),
      samples_(samples > 0 ? samples : 1),
      sampleDelayMs_(sampleDelayMs),
      minRaw_(minRaw),
      maxRaw_(maxRaw),
      resolutionBits_(resolutionBits) {}

void Potentiometer::begin() {
  pinMode(pin_, INPUT);
  analogReadResolution(resolutionBits_);
}

PotentiometerReading Potentiometer::read() const {
  long rawSum = 0;
  long mvSum = 0;

  for (int i = 0; i < samples_; i++) {
    rawSum += analogRead(pin_);
    mvSum += analogReadMilliVolts(pin_);
    if (sampleDelayMs_ > 0) {
      delay(sampleDelayMs_);
    }
  }

  int rawAvg = static_cast<int>(rawSum / samples_);
  int mvAvg = static_cast<int>(mvSum / samples_);

  return PotentiometerReading(rawAvg, mvAvg, minRaw_, maxRaw_);
}

void Potentiometer::setCalibration(int minRaw, int maxRaw) {
  minRaw_ = minRaw;
  maxRaw_ = maxRaw;
}

int Potentiometer::getMinRaw() const {
  return minRaw_;
}

int Potentiometer::getMaxRaw() const {
  return maxRaw_;
}

int Potentiometer::getMaxAdcValue() const {
  return (1 << resolutionBits_) - 1;
}
