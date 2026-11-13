#ifndef MOTOR_H
#define MOTOR_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <stdint.h>
#ifndef OUTPUT
#define OUTPUT 0x02
#endif
#ifndef LOW
#define LOW 0x0
#endif
#ifndef HIGH
#define HIGH 0x1
#endif
#endif

enum class MotorDirection {
  Stopped,
  Forward,
  Reverse
};

class Motor {
public:
  using DigitalWriteFn = void (*)(uint8_t pin, uint8_t val);
  using AnalogWriteFn = void (*)(uint8_t pin, int val);
  using PinModeFn = void (*)(uint8_t pin, uint8_t mode);

  static constexpr int kNoPwmPin = -1;

  Motor(uint8_t ain1Pin,
        uint8_t ain2Pin,
        int pwmPin = kNoPwmPin,
        uint8_t pwmResolutionBits = 8);

  void begin();

  void forward(float speedPercent = 100.0f);
  void reverse(float speedPercent = 100.0f);
  void stop();

  void setSpeedPercent(float speedPercent);
  void setModeChar(char mode, float speedPercent = 100.0f);

  MotorDirection getDirection() const;
  char getModeChar() const;
  bool isRunning() const;

  float getSpeedPercent() const;
  float getSpeedNormalized() const;
  int getPwmValue() const;
  int getMaxPwmValue() const;

  uint8_t getAin1Pin() const;
  uint8_t getAin2Pin() const;
  int getPwmPin() const;
  bool hasPwmPin() const;

  void setHardwareWriters(DigitalWriteFn digitalWriter,
                          AnalogWriteFn analogWriter = nullptr,
                          PinModeFn pinModeFn = nullptr);

private:
  void applyOutput();
  static float clampPercent(float speedPercent);

  uint8_t ain1Pin_;
  uint8_t ain2Pin_;
  int pwmPin_;
  uint8_t pwmResolutionBits_;
  MotorDirection direction_;
  float speedPercent_;
  DigitalWriteFn digitalWriter_;
  AnalogWriteFn analogWriter_;
  PinModeFn pinModeFn_;
};

#endif // MOTOR_H
