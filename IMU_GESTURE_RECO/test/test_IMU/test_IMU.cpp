#include <Arduino.h>
#include "unity.h"

#ifdef NANO33BLE_SENSE_REV2
  #include <Arduino_BMI270_BMM150.h>
#elif defined(NANO33BLE_SENSE)
  #include <Arduino_LSM9DS1.h>
#endif

void setUp(void) {
    // Procedure di setup
}

void tearDown(void) {
    // Procedure di pulizia
}

void test_inizialization(void) {
  TEST_ASSERT_TRUE_MESSAGE(IMU.begin(), "IMU initialization failed");
}

void test_function_fails(void) {
  TEST_ASSERT_TRUE_MESSAGE(1 == 0, "Fail message");
}

void test_equality(void) {
  TEST_ASSERT_EQUAL(1, 0);
}

void setup() {
  delay(5000);
  UNITY_BEGIN();
  RUN_TEST(test_inizialization);
  // RUN_TEST(test_function_fails); // Uncomment to see a failing test
  // RUN_TEST(test_equality);       // Uncomment to see a failing test
  UNITY_END();
}

void loop() {
    // Zona in loop, poco utilizzata in test su piattaforma Embedded
}