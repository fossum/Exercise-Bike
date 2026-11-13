#ifdef ARDUINO
#include <Arduino.h>
#endif
#include <unity.h>
#include "motor.h"

namespace {
constexpr size_t kMaxTrackedPins = 64;
int mockPinModes[kMaxTrackedPins];
int mockDigitalValues[kMaxTrackedPins];
int mockAnalogValues[kMaxTrackedPins];
int pinModeCallCount = 0;
int digitalWriteCallCount = 0;
int analogWriteCallCount = 0;

void mockPinMode(uint8_t pin, uint8_t mode) {
  if (pin < kMaxTrackedPins) {
    mockPinModes[pin] = mode;
  }
  pinModeCallCount++;
}

void mockDigitalWrite(uint8_t pin, uint8_t val) {
  if (pin < kMaxTrackedPins) {
    mockDigitalValues[pin] = val;
  }
  digitalWriteCallCount++;
}

void mockAnalogWrite(uint8_t pin, int val) {
  if (pin < kMaxTrackedPins) {
    mockAnalogValues[pin] = val;
  }
  analogWriteCallCount++;
}
} // namespace

void setUp(void) {
  for (size_t i = 0; i < kMaxTrackedPins; i++) {
    mockPinModes[i] = -1;
    mockDigitalValues[i] = -1;
    mockAnalogValues[i] = -1;
  }
  pinModeCallCount = 0;
  digitalWriteCallCount = 0;
  analogWriteCallCount = 0;
}

void tearDown(void) {}

void test_motor_initial_and_begin_state(void) {
  Motor motor(18, 19);
  motor.setHardwareWriters(mockDigitalWrite, mockAnalogWrite, mockPinMode);

  TEST_ASSERT_EQUAL_UINT8(18, motor.getAin1Pin());
  TEST_ASSERT_EQUAL_UINT8(19, motor.getAin2Pin());
  TEST_ASSERT_EQUAL_INT(Motor::kNoPwmPin, motor.getPwmPin());
  TEST_ASSERT_FALSE(motor.hasPwmPin());
  TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorDirection::Stopped),
                        static_cast<int>(motor.getDirection()));
  TEST_ASSERT_EQUAL_CHAR('s', motor.getModeChar());
  TEST_ASSERT_FALSE(motor.isRunning());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, motor.getSpeedPercent());

  motor.begin();

  TEST_ASSERT_EQUAL_INT(2, pinModeCallCount);
  TEST_ASSERT_EQUAL_INT(OUTPUT, mockPinModes[18]);
  TEST_ASSERT_EQUAL_INT(OUTPUT, mockPinModes[19]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[18]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[19]);
  TEST_ASSERT_EQUAL_INT(0, analogWriteCallCount);
}

void test_motor_forward_reverse_stop_digital(void) {
  Motor motor(18, 19);
  motor.setHardwareWriters(mockDigitalWrite, mockAnalogWrite, mockPinMode);
  motor.begin();

  motor.forward();
  TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorDirection::Forward),
                        static_cast<int>(motor.getDirection()));
  TEST_ASSERT_EQUAL_CHAR('f', motor.getModeChar());
  TEST_ASSERT_TRUE(motor.isRunning());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 100.0f, motor.getSpeedPercent());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, motor.getSpeedNormalized());
  TEST_ASSERT_EQUAL_INT(HIGH, mockDigitalValues[18]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[19]);

  motor.reverse();
  TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorDirection::Reverse),
                        static_cast<int>(motor.getDirection()));
  TEST_ASSERT_EQUAL_CHAR('r', motor.getModeChar());
  TEST_ASSERT_TRUE(motor.isRunning());
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[18]);
  TEST_ASSERT_EQUAL_INT(HIGH, mockDigitalValues[19]);

  motor.stop();
  TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorDirection::Stopped),
                        static_cast<int>(motor.getDirection()));
  TEST_ASSERT_EQUAL_CHAR('s', motor.getModeChar());
  TEST_ASSERT_FALSE(motor.isRunning());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, motor.getSpeedPercent());
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[18]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[19]);
}

void test_motor_pwm_speed_and_clamping(void) {
  Motor motor(18, 19, 20, 8);
  motor.setHardwareWriters(mockDigitalWrite, mockAnalogWrite, mockPinMode);
  motor.begin();

  TEST_ASSERT_TRUE(motor.hasPwmPin());
  TEST_ASSERT_EQUAL_INT(20, motor.getPwmPin());
  TEST_ASSERT_EQUAL_INT(255, motor.getMaxPwmValue());
  TEST_ASSERT_EQUAL_INT(3, pinModeCallCount);
  TEST_ASSERT_EQUAL_INT(OUTPUT, mockPinModes[20]);
  TEST_ASSERT_EQUAL_INT(0, mockAnalogValues[20]);

  motor.forward(50.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 50.0f, motor.getSpeedPercent());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, motor.getSpeedNormalized());
  TEST_ASSERT_EQUAL_INT(128, motor.getPwmValue());
  TEST_ASSERT_EQUAL_INT(128, mockAnalogValues[20]);
  TEST_ASSERT_EQUAL_INT(HIGH, mockDigitalValues[18]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[19]);

  motor.setSpeedPercent(150.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 100.0f, motor.getSpeedPercent());
  TEST_ASSERT_EQUAL_INT(255, motor.getPwmValue());
  TEST_ASSERT_EQUAL_INT(255, mockAnalogValues[20]);

  motor.reverse(-10.0f);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorDirection::Stopped),
                        static_cast<int>(motor.getDirection()));
  TEST_ASSERT_EQUAL_INT(0, motor.getPwmValue());
  TEST_ASSERT_EQUAL_INT(0, mockAnalogValues[20]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[18]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[19]);

  // Setting speed while stopped should keep motor stopped
  motor.setSpeedPercent(60.0f);
  TEST_ASSERT_FALSE(motor.isRunning());
  TEST_ASSERT_EQUAL_INT(0, mockAnalogValues[20]);
}

void test_motor_mode_char_helper(void) {
  Motor motor(18, 19, 20, 8);
  motor.setHardwareWriters(mockDigitalWrite, mockAnalogWrite, mockPinMode);
  motor.begin();

  motor.setModeChar('f', 75.0f);
  TEST_ASSERT_EQUAL_CHAR('f', motor.getModeChar());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 75.0f, motor.getSpeedPercent());
  TEST_ASSERT_EQUAL_INT(HIGH, mockDigitalValues[18]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[19]);
  TEST_ASSERT_EQUAL_INT(191, mockAnalogValues[20]);

  motor.setModeChar('x', 50.0f);
  TEST_ASSERT_EQUAL_CHAR('f', motor.getModeChar());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 75.0f, motor.getSpeedPercent());

  motor.setModeChar('r', 25.0f);
  TEST_ASSERT_EQUAL_CHAR('r', motor.getModeChar());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 25.0f, motor.getSpeedPercent());
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[18]);
  TEST_ASSERT_EQUAL_INT(HIGH, mockDigitalValues[19]);
  TEST_ASSERT_EQUAL_INT(64, mockAnalogValues[20]);

  motor.setModeChar('s');
  TEST_ASSERT_EQUAL_CHAR('s', motor.getModeChar());
  TEST_ASSERT_FALSE(motor.isRunning());
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[18]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[19]);
  TEST_ASSERT_EQUAL_INT(0, mockAnalogValues[20]);
}

#ifdef ARDUINO
void test_hardware_motor_pins(void) {
  Motor motor(18, 19);
  motor.begin();
  TEST_ASSERT_EQUAL_CHAR('s', motor.getModeChar());

  motor.forward();
  TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorDirection::Forward),
                        static_cast<int>(motor.getDirection()));

  motor.reverse();
  TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorDirection::Reverse),
                        static_cast<int>(motor.getDirection()));

  motor.stop();
  TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorDirection::Stopped),
                        static_cast<int>(motor.getDirection()));
}
#endif

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_motor_initial_and_begin_state);
  RUN_TEST(test_motor_forward_reverse_stop_digital);
  RUN_TEST(test_motor_pwm_speed_and_clamping);
  RUN_TEST(test_motor_mode_char_helper);
#ifdef ARDUINO
  RUN_TEST(test_hardware_motor_pins);
#endif
  return UNITY_END();
}

#ifdef ARDUINO
void setup() {
  delay(2000);
  runUnityTests();
}

void loop() {}
#else
int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  return runUnityTests();
}
#endif
