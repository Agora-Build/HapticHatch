#include "unity.h"
#include "rtsa_transport.h"

static float s_received_force = -1.0f;
static int   s_callback_count = 0;

static void test_rx_cb(const haptic_packet_t *pkt) {
    s_received_force = pkt->force;
    s_callback_count++;
}

TEST_CASE("transport_init returns ESP_OK", "[transport]") {
    TEST_ASSERT_EQUAL(ESP_OK, transport_init());
}

TEST_CASE("loopback: send triggers rx callback with same packet", "[transport]") {
    s_received_force = -1.0f;
    s_callback_count = 0;

    transport_init();
    transport_set_rx_callback(test_rx_cb);

    haptic_packet_t pkt = { .timestamp_ms = 1234, .force = 0.75f };
    TEST_ASSERT_EQUAL(ESP_OK, transport_send(&pkt));

    TEST_ASSERT_EQUAL_INT(1, s_callback_count);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.75f, s_received_force);
}

TEST_CASE("no callback set: transport_send returns ESP_OK without crash", "[transport]") {
    transport_init();
    transport_set_rx_callback(NULL);

    haptic_packet_t pkt = { .timestamp_ms = 0, .force = 0.5f };
    TEST_ASSERT_EQUAL(ESP_OK, transport_send(&pkt));
}

void app_main(void) {
    unity_run_all_tests();
}
