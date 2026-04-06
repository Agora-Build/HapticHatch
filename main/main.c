#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sensor.h"
#include "haptic.h"
#include "rtsa_transport.h"
#include "signaling_demo.h"
#include "sdkconfig.h"

static const char *TAG = "app";
static QueueHandle_t s_haptic_queue;

static void on_transport_rx(const haptic_packet_t *pkt) {
    /* Called from transport context — drop packet if queue is full to avoid
       latency buildup (older data is stale). */
    xQueueSend(s_haptic_queue, pkt, 0);
}

static void sensor_task(void *arg) {
    const TickType_t period_ticks =
        pdMS_TO_TICKS(1000 / CONFIG_SENSOR_SAMPLE_RATE_HZ);
    haptic_packet_t pkt;
    TickType_t last_wake = xTaskGetTickCount();

    while (1) {
        pkt.timestamp_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
        pkt.force        = sensor_read();
        transport_send(&pkt);
        vTaskDelayUntil(&last_wake, period_ticks);
    }
}

static void haptic_task(void *arg) {
    haptic_packet_t pkt;

    while (1) {
        if (xQueueReceive(s_haptic_queue, &pkt, portMAX_DELAY) == pdTRUE) {
            haptic_play(pkt.force);
        }
    }
}

void app_main(void) {
    ESP_ERROR_CHECK(sensor_init());
    ESP_ERROR_CHECK(haptic_init());

    s_haptic_queue = xQueueCreate(CONFIG_HAPTIC_QUEUE_DEPTH,
                                  sizeof(haptic_packet_t));
    configASSERT(s_haptic_queue);

    transport_set_rx_callback(on_transport_rx);
    ESP_ERROR_CHECK(transport_init());

    BaseType_t rc;
    rc = xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
    configASSERT(rc == pdPASS);
    rc = xTaskCreate(haptic_task, "haptic_task", 4096, NULL, 6, NULL);
    configASSERT(rc == pdPASS);

    ESP_LOGI(TAG, "HapticHatch started — sample_rate=%d Hz queue_depth=%d",
             CONFIG_SENSOR_SAMPLE_RATE_HZ, CONFIG_HAPTIC_QUEUE_DEPTH);

    signaling_demo_start();
}
