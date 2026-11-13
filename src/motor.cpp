#include "motor.h"

namespace {
void defaultDigitalWrite(uint8_t pin, uint8_t val) {
#ifdef ARDUINO
  digitalWrite(pin, val);
#else
  (void)pin;
  (void)val;
#endif
}

void defaultAnalogWrite(uint8_t pin, int val) {
#ifdef ARDUINO
  analogWrite(pin, val);
#else
  (void)pin;
  (void)val;
#endif
}

void defaultPinMode(uint8_t pin, uint8_t mode) {
#ifdef ARDUINO
  pinMode(pin, mode);
#else
  (void)pin;
  (void)mode;
#endif
}
} // namespace

Motor::Motor(uint8_t ain1Pin,
             uint8_t ain2Pin,
             int pwmPin,
             uint8_t pwmResolutionBits)
    : ain1Pin_(ain1Pin),
      ain2Pin_(ain2Pin),
      pwmPin_(pwmPin),
      pwmResolutionBits_(pwmResolutionBits > 0 ? pwmResolutionBits : 8),
      direction_(MotorDirection::Stopped),
      speedPercent_(0.0f),
      digitalWriter_(defaultDigitalWrite),
      analogWriter_(defaultAnalogWrite),
      pinModeFn_(defaultPinMode) {}

void Motor::begin() {
  if (pinModeFn_) {
    pinModeFn_(ain1Pin_, OUTPUT);
    pinModeFn_(ain2Pin_, OUTPUT);
    if (hasPwmPin()) {
      pinModeFn_(static_cast<uint8_t>(pwmPin_), OUTPUT);
    }
  }
  stop();
}

void Motor::forward(float speedPercent) {
  float clamped = clampPercent(speedPercent);
  if (clamped <= 0.0f) {
    stop();
    return;
  }
  direction_ = MotorDirection::Forward;
  speedPercent_ = clamped;
  applyOutput();
}

void Motor::reverse(float speedPercent) {
  float clamped = clampPercent(speedPercent);
  if (clamped <= 0.0f) {
    stop();
    return;
  }
  direction_ = MotorDirection::Reverse;
  speedPercent_ = clamped;
  applyOutput();
}

void Motor::stop() {
  direction_ = MotorDirection::Stopped;
  speedPercent_ = 0.0f;
  applyOutput();
}

void Motor::setSpeedPercent(float speedPercent) {
  float clamped = clampPercent(speedPercent);
  if (clamped <= 0.0f || direction_ == MotorDirection::Stopped) {
    stop();
    return;
  }
  speedPercent_ = clamped;
  applyOutput();
}

void Motor::setModeChar(char mode, float speedPercent) {
  if (mode == 'f') {
    forward(speedPercent);
  } else if (mode == 'r') {
    reverse(speedPercent);
  } else if (mode == 's') {
    stop();
  }
}

MotorDirection Motor::getDirection() const {
  return direction_;
}

char Motor::getModeChar() const {
  switch (direction_) {
    case MotorDirection::Forward:
      return 'f';
    case MotorDirection::Reverse:
      return 'r';
    case MotorDirection::Stopped:
    default:
      return 's';
  }
}

bool Motor::isRunning() const {
  return direction_ != MotorDirection::Stopped;
}

float Motor::getSpeedPercent() const {
  return speedPercent_;
}

float Motor::getSpeedNormalized() const {
  return speedPercent_ / 100.0f;
}

int Motor::getPwmValue() const {
  if (!isRunning()) {
    return 0;
  }
  return static_cast<int>((getSpeedNormalized() * static_cast<float>(getMaxPwmValue())) + 0.5f);
}

int Motor::getMaxPwmValue() const {
  return (1 << pwmResolutionBits_) - 1;
}

uint8_t Motor::getAin1Pin() const {
  return ain1Pin_;
}

uint8_t Motor::getAin2Pin() const {
  return ain2Pin_;
}

int Motor::getPwmPin() const {
  return pwmPin_;
}

bool Motor::hasPwmPin() const {
  return pwmPin_ >= 0;
}

void Motor::setHardwareWriters(DigitalWriteFn digitalWriter,
                               AnalogWriteFn analogWriter,
                               PinModeFn pinModeFn) {
  digitalWriter_ = digitalWriter ? digitalWriter : defaultDigitalWrite;
  analogWriter_ = analogWriter ? analogWriter : defaultAnalogWrite;
  pinModeFn_ = pinModeFn ? pinModeFn : defaultPinMode;
}

void Motor::applyOutput() {
  uint8_t ain1Val = LOW;
  uint8_t ain2Val = LOW;

  if (direction_ == MotorDirection::Forward) {
    ain1Val = HIGH;
    ain2Val = LOW;
  } else if (direction_ == MotorDirection::Reverse) {
    ain1Val = LOW;
    ain2Val = HIGH;
  }

  if (digitalWriter_) {
    digitalWriter_(ain1Pin_, ain1Val);
    digitalWriter_(ain2Pin_, ain2Val);
  }

  if (hasPwmPin() && analogWriter_) {
    analogWriter_(static_cast<uint8_t>(pwmPin_), getPwmValue());
  }
}

float Motor::clampPercent(float speedPercent) {
  if (speedPercent < 0.0f) {
    return 0.0f;
  }
  if (speedPercent > 100.0f) {
    return 100.0f;
  }
  return speedPercent;
}
