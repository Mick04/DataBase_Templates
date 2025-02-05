#include <Arduino.h>
#include <unity.h>

// Function prototypes
void test_relay_control(void);

void setup()
{
    // NOTE!!! Wait for >2 secs if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN(); // Start Unity test framework

    RUN_TEST(test_relay_control);

    UNITY_END(); // Stop Unity test framework
}

void loop()
{
    // Empty loop
}

void test_relay_control(void)
{
    // Add your test cases here
    // Example:
    // TEST_ASSERT_EQUAL(1, 1);
}