#ifdef ARDUINO
#include <Arduino.h>
#endif
#include <unity.h>
#include "potentiometer.h"

namespace {
int mockRawSequence[16];
int mockMvSequence[16];
int mockRawIndex = 0;
int mockMvIndex = 0;
int mockSequenceLength = 0;
uint8_t lastReadPin = 255;
int delayCallCount = 0;
uint32_t totalDelayedMs = 0;

int mockReadRaw(uint8_t pin) {
  lastReadPin = pin;
  int val = mockRawSequence[mockRawIndex % mockSequenceLength];
  mockRawIndex++;
  return val;
}

int mockReadMv(uint8_t pin) {
  lastReadPin = pin;
  int val = mockMvSequence[mockMvIndex % mockSequenceLength];
  mockMvIndex++;
  return val;
}

void mockDelay(uint32_t ms) {
  delayCallCount++;
  totalDelayedMs += ms;
}
} // namespace

void setUp(void) {
  mockRawIndex = 0;
  mockMvIndex = 0;
  mockSequenceLength = 1;
  mockRawSequence[0] = 0;
  mockMvSequence[0] = 0;
  lastReadPin = 255;
  delayCallCount = 0;
  totalDelayedMs = 0;
}

void tearDown(void) {}

void test_reading_getters_basic(void) {
  PotentiometerReading reading(2048, 1650, 0, 4096);
  TEST_ASSERT_EQUAL_INT(2048, reading.getRaw());
  TEST_ASSERT_EQUAL_INT(1650, reading.getMilliVolts());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.65f, reading.getVolts());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, reading.getNormalized());
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.0f, reading.getPercent());
}

void test_reading_calibration_and_clamping(void) {
  // Custom range: min = 500, max = 3500 (span = 3000)
  PotentiometerReading mid(2000, 1600, 500, 3500);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, mid.getNormalized());
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.0f, mid.getPercent());

  // Below min should clamp to 0.0 / 0%
  PotentiometerReading belowMin(200, 150, 500, 3500);
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, belowMin.getNormalized());
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, belowMin.getPercent());

  // Above max should clamp to 1.0 / 100%
  PotentiometerReading aboveMax(3900, 3100, 500, 3500);
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, aboveMax.getNormalized());
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 100.0f, aboveMax.getPercent());

  // Invalid calibration (max <= min) should safely return 0.0
  PotentiometerReading invalidBounds(2000, 1600, 3000, 1000);
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, invalidBounds.getNormalized());
}

void test_potentiometer_averaging_and_delays(void) {
  mockSequenceLength = 4;
  mockRawSequence[0] = 1000;
  mockRawSequence[1] = 1010;
  mockRawSequence[2] = 1020;
  mockRawSequence[3] = 1030;

  mockMvSequence[0] = 800;
  mockMvSequence[1] = 810;
  mockMvSequence[2] = 820;
  mockMvSequence[3] = 830;

  Potentiometer pot(4, 4, 3, 0, 4000, 12);
  pot.setHardwareReaders(mockReadRaw, mockReadMv, mockDelay);

  PotentiometerReading reading = pot.read();

  TEST_ASSERT_EQUAL_UINT8(4, lastReadPin);
  TEST_ASSERT_EQUAL_INT(1015, reading.getRaw());
  TEST_ASSERT_EQUAL_INT(815, reading.getMilliVolts());
  TEST_ASSERT_EQUAL_INT(4, delayCallCount);
  TEST_ASSERT_EQUAL_UINT32(12, totalDelayedMs);
  TEST_ASSERT_EQUAL_INT(4095, pot.getMaxAdcValue());
}

void test_potentiometer_calibration_update(void) {
  mockSequenceLength = 1;
  mockRawSequence[0] = 1500;
  mockMvSequence[0] = 1200;

  Potentiometer pot(4, 1, 0, 0, 4095, 12);
  pot.setHardwareReaders(mockReadRaw, mockReadMv, mockDelay);
  pot.setCalibration(1000, 2000);

  TEST_ASSERT_EQUAL_INT(1000, pot.getMinRaw());
  TEST_ASSERT_EQUAL_INT(2000, pot.getMaxRaw());

  PotentiometerReading reading = pot.read();
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, reading.getNormalized());
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.0f, reading.getPercent());
}

#ifdef ARDUINO
void test_hardware_live_adc_read(void) {
  Potentiometer pot(4, 8, 1, 0, 4095, 12);
  pot.begin();
  PotentiometerReading reading = pot.read();

  TEST_ASSERT_GREATER_OR_EQUAL_INT(0, reading.getRaw());
  TEST_ASSERT_LESS_OR_EQUAL_INT(4095, reading.getRaw());
  TEST_ASSERT_GREATER_OR_EQUAL_INT(0, reading.getMilliVolts());
  TEST_ASSERT_LESS_OR_EQUAL_INT(3600, reading.getMilliVolts());
  TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(0.0f, reading.getPercent());
  TEST_ASSERT_LESS_OR_EQUAL_FLOAT(100.0f, reading.getPercent());
}
#endif

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_reading_getters_basic);
  RUN_TEST(test_reading_calibration_and_clamping);
  RUN_TEST(test_potentiometer_averaging_and_delays);
  RUN_TEST(test_potentiometer_calibration_update);
#ifdef ARDUINO
  RUN_TEST(test_hardware_live_adc_read);
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
