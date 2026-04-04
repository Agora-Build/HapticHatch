#include "unity.h"
#include "rtsa_transport.h"

static float    s_received_force     = -1.0f;
static uint32_t s_received_timestamp = 0;
static int      s_callback_count     = 0;

static void test_rx_cb(const haptic_packet_t *pkt) {
    s_received_force     = pkt->force;
    s_received_timestamp = pkt->timestamp_ms;
    s_callback_count++;
}

TEST_CASE("transport_init returns ESP_OK", "[transport]") {
    TEST_ASSERT_EQUAL(ESP_OK, transport_init());
}

TEST_CASE("loopback: send triggers rx callback with same packet", "[transport]") {
    s_received_force     = -1.0f;
    s_received_timestamp = 0;
    s_callback_count     = 0;

    transport_init();
    transport_set_rx_callback(test_rx_cb);

    haptic_packet_t pkt = { .timestamp_ms = 1234, .force = 0.75f };
    TEST_ASSERT_EQUAL(ESP_OK, transport_send(&pkt));

    TEST_ASSERT_EQUAL_INT(1, s_callback_count);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.75f, s_received_force);
    TEST_ASSERT_EQUAL_UINT32(1234, s_received_timestamp);
}

TEST_CASE("no callback set: transport_send returns ESP_OK without crash", "[transport]") {
    transport_init();
    transport_set_rx_callback(NULL);

    haptic_packet_t pkt = { .timestamp_ms = 0, .force = 0.5f };
    TEST_ASSERT_EQUAL(ESP_OK, transport_send(&pkt));
}

TEST_CASE("deregister callback: send after NULL does not fire old callback", "[transport]") {
    s_callback_count = 0;

    transport_init();
    transport_set_rx_callback(test_rx_cb);  /* register */
    transport_set_rx_callback(NULL);         /* deregister */

    haptic_packet_t pkt = { .timestamp_ms = 0, .force = 0.5f };
    TEST_ASSERT_EQUAL(ESP_OK, transport_send(&pkt));

    TEST_ASSERT_EQUAL_INT(0, s_callback_count);  /* old callback must not fire */
}

void app_main(void) {
    unity_run_all_tests();
}
