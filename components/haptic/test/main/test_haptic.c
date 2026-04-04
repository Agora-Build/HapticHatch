#include "unity.h"
#include "haptic.h"

TEST_CASE("haptic_init returns ESP_OK", "[haptic]") {
    TEST_ASSERT_EQUAL(ESP_OK, haptic_init());
}

TEST_CASE("haptic_play does not crash for full range", "[haptic]") {
    haptic_init();
    /* Boundary and midpoint — stub must not crash or assert */
    haptic_play(0.0f);
    haptic_play(0.5f);
    haptic_play(1.0f);
}

void app_main(void) {
    unity_run_all_tests();
}
