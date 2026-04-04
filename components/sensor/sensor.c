#include "sensor.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <math.h>

static const char *TAG = "sensor";

esp_err_t sensor_init(void) {
    ESP_LOGI(TAG, "sensor init (stub: sine wave)");
    return ESP_OK;
}

float sensor_read(void) {
    /* Stub: 1 Hz sine wave mapped to [0.0, 1.0] */
    int64_t us = esp_timer_get_time();
    float t = (float)(us % 1000000) / 1000000.0f;
    return (sinf(2.0f * (float)M_PI * t) + 1.0f) / 2.0f;
}
