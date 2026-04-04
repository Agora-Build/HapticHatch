#include "haptic.h"
#include "esp_log.h"

static const char *TAG = "haptic";

esp_err_t haptic_init(void) {
    ESP_LOGI(TAG, "haptic init (stub: console log)");
    return ESP_OK;
}

void haptic_play(float intensity) {
    ESP_LOGI(TAG, "play intensity=%.3f", intensity);
}
