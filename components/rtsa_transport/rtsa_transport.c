#include "rtsa_transport.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "rtsa_transport";
static transport_rx_cb_t s_rx_cb = NULL;

esp_err_t transport_init(void) {
    ESP_LOGI(TAG, "transport init (stub: loopback, channel='%s')",
             CONFIG_RTSA_CHANNEL_ID);
    return ESP_OK;
}

esp_err_t transport_send(const haptic_packet_t *pkt) {
    if (s_rx_cb) {
        s_rx_cb(pkt);
    }
    return ESP_OK;
}

void transport_set_rx_callback(transport_rx_cb_t cb) {
    s_rx_cb = cb;
}
