#include "unity.h"
#include "sensor.h"

TEST_CASE("sensor_init returns ESP_OK", "[sensor]") {
    TEST_ASSERT_EQUAL(ESP_OK, sensor_init());
}

TEST_CASE("sensor_read returns value in [0.0, 1.0]", "[sensor]") {
    sensor_init();
    for (int i = 0; i < 10; i++) {
        float val = sensor_read();
        /* Range check: value must be in [0.0, 1.0], i.e. within 0.5 of centre 0.5 */
        TEST_ASSERT_FLOAT_WITHIN(0.5f, 0.5f, val);
    }
}

void app_main(void) {
    unity_run_all_tests();
}
