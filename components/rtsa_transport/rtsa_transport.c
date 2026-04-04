#include "rtsa_transport.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "sdkconfig.h"

static const char *TAG = "rtsa_transport";

/* Spinlock guards s_rx_cb against concurrent write (transport_set_rx_callback)
   and read (transport_send / SDK receive thread). */
static portMUX_TYPE      s_rx_cb_mux = portMUX_INITIALIZER_UNLOCKED;
static transport_rx_cb_t s_rx_cb     = NULL;

esp_err_t transport_init(void) {
    ESP_LOGI(TAG, "transport init (stub: loopback, channel='%s')",
             CONFIG_RTSA_CHANNEL_ID);
    return ESP_OK;
}

esp_err_t transport_send(const haptic_packet_t *pkt) {
    transport_rx_cb_t cb;
    taskENTER_CRITICAL(&s_rx_cb_mux);
    cb = s_rx_cb;
    taskEXIT_CRITICAL(&s_rx_cb_mux);
    if (cb) {
        cb(pkt);
    }
    return ESP_OK;
}

void transport_set_rx_callback(transport_rx_cb_t cb) {
    taskENTER_CRITICAL(&s_rx_cb_mux);
    s_rx_cb = cb;
    taskEXIT_CRITICAL(&s_rx_cb_mux);
}
