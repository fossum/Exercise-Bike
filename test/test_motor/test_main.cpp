#ifdef ARDUINO
#include <Arduino.h>
#endif
#include <unity.h>
#include "motor.h"
#include "pins.h"

namespace {
constexpr size_t kMaxTrackedPins = 64;
int mockPinModes[kMaxTrackedPins];
int mockDigitalValues[kMaxTrackedPins];
int mockAnalogValues[kMaxTrackedPins];
int pinModeCallCount = 0;
int digitalWriteCallCount = 0;
int analogWriteCallCount = 0;
bool sawBothDirectionPinsHigh = false;

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
  if (mockDigitalValues[PIN_AIN1] == HIGH && mockDigitalValues[PIN_AIN2] == HIGH) {
    sawBothDirectionPinsHigh = true;
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
  sawBothDirectionPinsHigh = false;
}

void tearDown(void) {}

void test_motor_initial_and_begin_state(void) {
  Motor motor(PIN_AIN1, PIN_AIN2);
  motor.setHardwareWriters(mockDigitalWrite, mockAnalogWrite, mockPinMode);

  TEST_ASSERT_EQUAL_UINT8(PIN_AIN1, motor.getAin1Pin());
  TEST_ASSERT_EQUAL_UINT8(PIN_AIN2, motor.getAin2Pin());
  TEST_ASSERT_EQUAL_INT(Motor::kNoPwmPin, motor.getPwmPin());
  TEST_ASSERT_FALSE(motor.hasPwmPin());
  TEST_ASSERT_EQUAL_INT(Motor::kNoStandbyPin, motor.getStandbyPin());
  TEST_ASSERT_FALSE(motor.hasStandbyPin());
  TEST_ASSERT_FALSE(motor.isInStandby());
  TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorDirection::Stopped),
                        static_cast<int>(motor.getDirection()));
  TEST_ASSERT_EQUAL_CHAR('s', motor.getModeChar());
  TEST_ASSERT_FALSE(motor.isRunning());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, motor.getSpeedPercent());

  motor.begin();

  TEST_ASSERT_EQUAL_INT(2, pinModeCallCount);
  TEST_ASSERT_EQUAL_INT(OUTPUT, mockPinModes[PIN_AIN1]);
  TEST_ASSERT_EQUAL_INT(OUTPUT, mockPinModes[PIN_AIN2]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN1]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN2]);
  TEST_ASSERT_EQUAL_INT(0, analogWriteCallCount);
}

void test_motor_forward_reverse_stop_digital(void) {
  Motor motor(PIN_AIN1, PIN_AIN2);
  motor.setHardwareWriters(mockDigitalWrite, mockAnalogWrite, mockPinMode);
  motor.begin();

  motor.forward();
  TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorDirection::Forward),
                        static_cast<int>(motor.getDirection()));
  TEST_ASSERT_EQUAL_CHAR('f', motor.getModeChar());
  TEST_ASSERT_TRUE(motor.isRunning());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 100.0f, motor.getSpeedPercent());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, motor.getSpeedNormalized());
  TEST_ASSERT_EQUAL_INT(HIGH, mockDigitalValues[PIN_AIN1]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN2]);

  motor.reverse();
  TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorDirection::Reverse),
                        static_cast<int>(motor.getDirection()));
  TEST_ASSERT_EQUAL_CHAR('r', motor.getModeChar());
  TEST_ASSERT_TRUE(motor.isRunning());
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN1]);
  TEST_ASSERT_EQUAL_INT(HIGH, mockDigitalValues[PIN_AIN2]);

  motor.stop();
  TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorDirection::Stopped),
                        static_cast<int>(motor.getDirection()));
  TEST_ASSERT_EQUAL_CHAR('s', motor.getModeChar());
  TEST_ASSERT_FALSE(motor.isRunning());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, motor.getSpeedPercent());
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN1]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN2]);
}

void test_motor_pwm_speed_and_clamping(void) {
  Motor motor(PIN_AIN1, PIN_AIN2, PIN_PWM, 8);
  motor.setHardwareWriters(mockDigitalWrite, mockAnalogWrite, mockPinMode);
  motor.begin();

  TEST_ASSERT_TRUE(motor.hasPwmPin());
  TEST_ASSERT_EQUAL_INT(PIN_PWM, motor.getPwmPin());
  TEST_ASSERT_EQUAL_INT(255, motor.getMaxPwmValue());
  TEST_ASSERT_EQUAL_INT(3, pinModeCallCount);
  TEST_ASSERT_EQUAL_INT(OUTPUT, mockPinModes[PIN_PWM]);
  TEST_ASSERT_EQUAL_INT(0, mockAnalogValues[PIN_PWM]);

  motor.forward(50.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 50.0f, motor.getSpeedPercent());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, motor.getSpeedNormalized());
  TEST_ASSERT_EQUAL_INT(128, motor.getPwmValue());
  TEST_ASSERT_EQUAL_INT(128, mockAnalogValues[PIN_PWM]);
  TEST_ASSERT_EQUAL_INT(HIGH, mockDigitalValues[PIN_AIN1]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN2]);

  motor.setSpeedPercent(150.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 100.0f, motor.getSpeedPercent());
  TEST_ASSERT_EQUAL_INT(255, motor.getPwmValue());
  TEST_ASSERT_EQUAL_INT(255, mockAnalogValues[PIN_PWM]);

  motor.reverse(-10.0f);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorDirection::Stopped),
                        static_cast<int>(motor.getDirection()));
  TEST_ASSERT_EQUAL_INT(0, motor.getPwmValue());
  TEST_ASSERT_EQUAL_INT(0, mockAnalogValues[PIN_PWM]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN1]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN2]);

  // Setting speed while stopped should keep motor stopped
  motor.setSpeedPercent(60.0f);
  TEST_ASSERT_FALSE(motor.isRunning());
  TEST_ASSERT_EQUAL_INT(0, mockAnalogValues[PIN_PWM]);
}

void test_motor_mode_char_helper(void) {
  Motor motor(PIN_AIN1, PIN_AIN2, PIN_PWM, 8);
  motor.setHardwareWriters(mockDigitalWrite, mockAnalogWrite, mockPinMode);
  motor.begin();

  motor.setModeChar('f', 75.0f);
  TEST_ASSERT_EQUAL_CHAR('f', motor.getModeChar());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 75.0f, motor.getSpeedPercent());
  TEST_ASSERT_EQUAL_INT(HIGH, mockDigitalValues[PIN_AIN1]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN2]);
  TEST_ASSERT_EQUAL_INT(191, mockAnalogValues[PIN_PWM]);

  motor.setModeChar('x', 50.0f);
  TEST_ASSERT_EQUAL_CHAR('f', motor.getModeChar());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 75.0f, motor.getSpeedPercent());

  motor.setModeChar('r', 25.0f);
  TEST_ASSERT_EQUAL_CHAR('r', motor.getModeChar());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 25.0f, motor.getSpeedPercent());
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN1]);
  TEST_ASSERT_EQUAL_INT(HIGH, mockDigitalValues[PIN_AIN2]);
  TEST_ASSERT_EQUAL_INT(64, mockAnalogValues[PIN_PWM]);

  motor.setModeChar('s');
  TEST_ASSERT_EQUAL_CHAR('s', motor.getModeChar());
  TEST_ASSERT_FALSE(motor.isRunning());
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN1]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN2]);
  TEST_ASSERT_EQUAL_INT(0, mockAnalogValues[PIN_PWM]);
}

void test_motor_standby_pin_control(void) {
  Motor motor(PIN_AIN1, PIN_AIN2, PIN_PWM, 8, PIN_STBY);
  motor.setHardwareWriters(mockDigitalWrite, mockAnalogWrite, mockPinMode);

  TEST_ASSERT_TRUE(motor.hasPwmPin());
  TEST_ASSERT_EQUAL_INT(PIN_PWM, motor.getPwmPin());
  TEST_ASSERT_TRUE(motor.hasStandbyPin());
  TEST_ASSERT_EQUAL_INT(PIN_STBY, motor.getStandbyPin());
  TEST_ASSERT_TRUE(motor.isInStandby());

  motor.begin();
  TEST_ASSERT_EQUAL_INT(4, pinModeCallCount);
  TEST_ASSERT_EQUAL_INT(OUTPUT, mockPinModes[PIN_AIN1]);
  TEST_ASSERT_EQUAL_INT(OUTPUT, mockPinModes[PIN_AIN2]);
  TEST_ASSERT_EQUAL_INT(OUTPUT, mockPinModes[PIN_PWM]);
  TEST_ASSERT_EQUAL_INT(OUTPUT, mockPinModes[PIN_STBY]);
  TEST_ASSERT_EQUAL_INT(0, mockAnalogValues[PIN_PWM]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_STBY]);
  TEST_ASSERT_TRUE(motor.isInStandby());

  motor.forward(80.0f);
  TEST_ASSERT_EQUAL_INT(HIGH, mockDigitalValues[PIN_AIN1]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN2]);
  TEST_ASSERT_EQUAL_INT(204, mockAnalogValues[PIN_PWM]);
  TEST_ASSERT_EQUAL_INT(HIGH, mockDigitalValues[PIN_STBY]);
  TEST_ASSERT_FALSE(motor.isInStandby());

  motor.reverse(80.0f);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN1]);
  TEST_ASSERT_EQUAL_INT(HIGH, mockDigitalValues[PIN_AIN2]);
  TEST_ASSERT_EQUAL_INT(204, mockAnalogValues[PIN_PWM]);
  TEST_ASSERT_EQUAL_INT(HIGH, mockDigitalValues[PIN_STBY]);
  TEST_ASSERT_FALSE(motor.isInStandby());

  motor.stop();
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN1]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_AIN2]);
  TEST_ASSERT_EQUAL_INT(0, mockAnalogValues[PIN_PWM]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_STBY]);
  TEST_ASSERT_TRUE(motor.isInStandby());

  motor.setModeChar('f', 80.0f);
  TEST_ASSERT_EQUAL_INT(204, mockAnalogValues[PIN_PWM]);
  TEST_ASSERT_EQUAL_INT(HIGH, mockDigitalValues[PIN_STBY]);
  TEST_ASSERT_FALSE(motor.isInStandby());

  motor.setSpeedPercent(0.0f);
  TEST_ASSERT_EQUAL_INT(0, mockAnalogValues[PIN_PWM]);
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_STBY]);
  TEST_ASSERT_TRUE(motor.isInStandby());
}

void test_motor_skips_redundant_output_writes(void) {
  Motor motor(PIN_AIN1, PIN_AIN2, PIN_PWM, 8, PIN_STBY);
  motor.setHardwareWriters(mockDigitalWrite, mockAnalogWrite, mockPinMode);

  // begin() MUST unconditionally write all outputs even from default Stopped/0% state
  motor.begin();
  TEST_ASSERT_EQUAL_INT(3, digitalWriteCallCount);
  TEST_ASSERT_EQUAL_INT(1, analogWriteCallCount);

  // Redundant stop and stopped-state commands emit zero writes
  motor.stop();
  motor.forward(0.0f);
  motor.reverse(-5.0f);
  motor.setSpeedPercent(60.0f);
  motor.setModeChar('s');
  TEST_ASSERT_EQUAL_INT(3, digitalWriteCallCount);
  TEST_ASSERT_EQUAL_INT(1, analogWriteCallCount);

  // Transition to Forward 80% writes outputs once
  motor.forward(80.0f);
  TEST_ASSERT_EQUAL_INT(6, digitalWriteCallCount);
  TEST_ASSERT_EQUAL_INT(2, analogWriteCallCount);

  // Redundant Forward / speed / mode-char commands emit zero writes
  motor.forward(80.0f);
  motor.setSpeedPercent(80.0f);
  motor.setModeChar('f', 80.0f);
  TEST_ASSERT_EQUAL_INT(6, digitalWriteCallCount);
  TEST_ASSERT_EQUAL_INT(2, analogWriteCallCount);

  // Actual speed change to 100% writes outputs once
  motor.setSpeedPercent(100.0f);
  TEST_ASSERT_EQUAL_INT(9, digitalWriteCallCount);
  TEST_ASSERT_EQUAL_INT(3, analogWriteCallCount);

  // Clamped-equivalent speeds (150% -> 100%) emit zero writes
  motor.setSpeedPercent(150.0f);
  motor.forward(150.0f);
  TEST_ASSERT_EQUAL_INT(9, digitalWriteCallCount);
  TEST_ASSERT_EQUAL_INT(3, analogWriteCallCount);

  // Direction change to Reverse 50% writes outputs once
  motor.reverse(50.0f);
  TEST_ASSERT_EQUAL_INT(12, digitalWriteCallCount);
  TEST_ASSERT_EQUAL_INT(4, analogWriteCallCount);

  // Redundant Reverse / speed / mode-char commands emit zero writes
  motor.reverse(50.0f);
  motor.setSpeedPercent(50.0f);
  motor.setModeChar('r', 50.0f);
  TEST_ASSERT_EQUAL_INT(12, digitalWriteCallCount);
  TEST_ASSERT_EQUAL_INT(4, analogWriteCallCount);

  // Actual speed change while reversing writes outputs once
  motor.setSpeedPercent(75.0f);
  TEST_ASSERT_EQUAL_INT(15, digitalWriteCallCount);
  TEST_ASSERT_EQUAL_INT(5, analogWriteCallCount);

  // Transition to Stopped writes outputs once; repeated stop emits zero writes
  motor.stop();
  TEST_ASSERT_EQUAL_INT(18, digitalWriteCallCount);
  TEST_ASSERT_EQUAL_INT(6, analogWriteCallCount);
  motor.stop();
  TEST_ASSERT_EQUAL_INT(18, digitalWriteCallCount);
  TEST_ASSERT_EQUAL_INT(6, analogWriteCallCount);

  // Re-calling begin() while already Stopped still unconditionally writes outputs
  motor.begin();
  TEST_ASSERT_EQUAL_INT(21, digitalWriteCallCount);
  TEST_ASSERT_EQUAL_INT(7, analogWriteCallCount);
}

void test_motor_edge_cases_and_safe_reversal(void) {
  // 1. Standby without PWM (digital-only mode with standby pin) & zero pwmResolutionBits fallback to 8-bit
  Motor digitalStandbyMotor(PIN_AIN1, PIN_AIN2, Motor::kNoPwmPin, 0, PIN_STBY);
  digitalStandbyMotor.setHardwareWriters(mockDigitalWrite, mockAnalogWrite, mockPinMode);
  TEST_ASSERT_FALSE(digitalStandbyMotor.hasPwmPin());
  TEST_ASSERT_TRUE(digitalStandbyMotor.hasStandbyPin());
  TEST_ASSERT_EQUAL_INT(255, digitalStandbyMotor.getMaxPwmValue());

  digitalStandbyMotor.begin();
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_STBY]);
  TEST_ASSERT_EQUAL_INT(0, analogWriteCallCount);

  digitalStandbyMotor.forward(100.0f);
  TEST_ASSERT_EQUAL_INT(HIGH, mockDigitalValues[PIN_STBY]);
  TEST_ASSERT_EQUAL_INT(0, analogWriteCallCount);

  // 2. forward(0.0f) while running transitions to Stopped + standby LOW
  digitalStandbyMotor.forward(0.0f);
  TEST_ASSERT_FALSE(digitalStandbyMotor.isRunning());
  TEST_ASSERT_TRUE(digitalStandbyMotor.isInStandby());
  TEST_ASSERT_EQUAL_INT(LOW, mockDigitalValues[PIN_STBY]);

  // 3. Direct Reverse -> Forward transition never transiently drives both AIN1 and AIN2 HIGH
  sawBothDirectionPinsHigh = false;
  digitalStandbyMotor.reverse(100.0f);
  digitalStandbyMotor.forward(100.0f);
  digitalStandbyMotor.reverse(100.0f);
  TEST_ASSERT_FALSE(sawBothDirectionPinsHigh);

  // 4. Custom 10-bit PWM resolution and nullptr writer fallback
  Motor pwm10Motor(PIN_AIN1, PIN_AIN2, PIN_PWM, 10);
  TEST_ASSERT_EQUAL_INT(1023, pwm10Motor.getMaxPwmValue());
  pwm10Motor.setHardwareWriters(nullptr, nullptr, nullptr);
  pwm10Motor.begin();
  pwm10Motor.forward(50.0f);
  TEST_ASSERT_EQUAL_INT(512, pwm10Motor.getPwmValue());
}

#ifdef ARDUINO
void test_hardware_motor_pins(void) {
  Motor motor(PIN_AIN1, PIN_AIN2, PIN_PWM, 8, PIN_STBY);
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
  RUN_TEST(test_motor_standby_pin_control);
  RUN_TEST(test_motor_skips_redundant_output_writes);
  RUN_TEST(test_motor_edge_cases_and_safe_reversal);
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
