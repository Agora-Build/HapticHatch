#include "unity.h"
#include "sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TEST_CASE("sensor_init returns ESP_OK", "[sensor]") {
    TEST_ASSERT_EQUAL(ESP_OK, sensor_init());
}

TEST_CASE("sensor_read returns value in [0.0, 1.0] across full cycle", "[sensor]") {
    sensor_init();
    float min_seen = 1.0f, max_seen = 0.0f;
    /* Sample 100 times at 10 ms intervals = 1000 ms = one full 1 Hz period */
    for (int i = 0; i < 100; i++) {
        float val = sensor_read();
        TEST_ASSERT_FLOAT_WITHIN(0.5f, 0.5f, val); /* in [0.0, 1.0] */
        if (val < min_seen) min_seen = val;
        if (val > max_seen) max_seen = val;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    /* Verify the sine wave actually swings through most of its range */
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, min_seen); /* approached 0 */
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 1.0f, max_seen); /* approached 1 */
}

void app_main(void) {
    unity_run_all_tests();
}
