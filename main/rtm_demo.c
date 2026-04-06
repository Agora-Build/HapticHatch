#include "rtm_demo.h"

#include "agora_rtc_api.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include <string.h>

static const char *TAG = "rtm_demo";

/* EventGroup bits */
#define WIFI_CONNECTED_BIT BIT0
#define RTM_LOGGED_IN_BIT  BIT1

static EventGroupHandle_t s_events;

/* ------------------------------------------------------------------ */
/* Message payload                                                      */
/* ------------------------------------------------------------------ */
typedef struct {
    uint32_t seq;
    uint32_t timestamp_ms;
    float    force;
} __attribute__((packed)) rtm_msg_t;

/* ------------------------------------------------------------------ */
/* WiFi                                                                 */
/* ------------------------------------------------------------------ */
static void wifi_event_handler(void *arg, esp_event_base_t base,
                               int32_t id, void *data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi disconnected — reconnecting");
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *ev = (ip_event_got_ip_t *)data;
        ESP_LOGI(TAG, "WiFi connected, IP: " IPSTR, IP2STR(&ev->ip_info.ip));
        xEventGroupSetBits(s_events, WIFI_CONNECTED_BIT);
    }
}

static void wifi_init(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                               &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                               &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {0};
    strlcpy((char *)wifi_config.sta.ssid,     CONFIG_DEMO_WIFI_SSID,
            sizeof(wifi_config.sta.ssid));
    strlcpy((char *)wifi_config.sta.password, CONFIG_DEMO_WIFI_PASSWORD,
            sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
}

/* ------------------------------------------------------------------ */
/* Agora RTC event handler (minimal — only errors logged)              */
/* ------------------------------------------------------------------ */
static void on_rtc_error(connection_id_t conn_id, int code, const char *msg)
{
    ESP_LOGE(TAG, "RTC error conn=%u code=%d: %s",
             (unsigned)conn_id, code, msg ? msg : "");
}

static const agora_rtc_event_handler_t s_rtc_handler = {
    .on_error = on_rtc_error,
};

/* ------------------------------------------------------------------ */
/* Agora Signaling callbacks                                            */
/* ------------------------------------------------------------------ */
static void on_rtm_event(const char *rtm_uid, rtm_event_type_e event_type,
                         rtm_err_code_e err_code)
{
    if (event_type == RTM_EVENT_TYPE_LOGIN) {
        if (err_code == ERR_RTM_OK) {
            ESP_LOGI(TAG, "Signaling login success uid=%s", rtm_uid);
            xEventGroupSetBits(s_events, RTM_LOGGED_IN_BIT);
        } else {
            ESP_LOGE(TAG, "Signaling login failed uid=%s err=%d", rtm_uid, err_code);
        }
    } else if (event_type == RTM_EVENT_TYPE_KICKOFF) {
        ESP_LOGW(TAG, "Signaling kicked off uid=%s err=%d", rtm_uid, err_code);
    }
}

static void on_rtm_data(const char *rtm_uid, const void *msg, size_t msg_len,
                        const char *custom_type)
{
    if (msg_len == sizeof(rtm_msg_t)) {
        rtm_msg_t pkt;
        memcpy(&pkt, msg, sizeof(pkt));
        ESP_LOGI(TAG, "RX from=%-12s seq=%-6u ts=%u ms force=%.3f type=%s",
                 rtm_uid, (unsigned)pkt.seq, (unsigned)pkt.timestamp_ms,
                 pkt.force, custom_type ? custom_type : "");
    } else {
        ESP_LOGW(TAG, "RX from=%s unexpected len=%u", rtm_uid, (unsigned)msg_len);
    }
}

static void on_rtm_send_data_result(const char *rtm_uid, uint32_t msg_id,
                                    rtm_msg_state_e state)
{
    /* RTM_MSG_STATE_RECEIVED is the happy path; log anything else. */
    if (state != RTM_MSG_STATE_RECEIVED) {
        ESP_LOGW(TAG, "TX result to=%s msg_id=%u state=%d",
                 rtm_uid, (unsigned)msg_id, state);
    }
}

static const agora_rtm_handler_t s_rtm_handler = {
    .on_rtm_event            = on_rtm_event,
    .on_rtm_data             = on_rtm_data,
    .on_rtm_send_data_result = on_rtm_send_data_result,
};

/* ------------------------------------------------------------------ */
/* Send task — 100 Hz; SDK caps at 60 qps                              */
/* ------------------------------------------------------------------ */
static void rtm_send_task(void *arg)
{
    const TickType_t period = pdMS_TO_TICKS(10); /* 100 Hz */
    TickType_t       last   = xTaskGetTickCount();
    uint32_t         seq    = 0;

    while (1) {
        rtm_msg_t msg = {
            .seq          = seq,
            .timestamp_ms = (uint32_t)(esp_timer_get_time() / 1000ULL),
            /* force ramps 0.00 → 0.99 cyclically */
            .force        = (float)(seq % 100) / 100.0f,
        };

        int ret = agora_rtc_send_rtm_data(CONFIG_AGORA_RTM_REMOTE_UID,
                                          &msg, sizeof(msg),
                                          seq, "haptic");
        if (ret == 0) {
            ESP_LOGI(TAG, "TX seq=%-6u ts=%u ms force=%.2f",
                     (unsigned)seq, (unsigned)msg.timestamp_ms, msg.force);
        } else {
            /* ERR_RTM_EXCEED_MSG_CNT (1005) is expected above 60 qps */
            ESP_LOGW(TAG, "TX FAIL seq=%-6u err=%d (%s)",
                     (unsigned)seq, ret, agora_rtc_err_2_str(ret));
        }

        seq++;
        vTaskDelayUntil(&last, period);
    }
}

/* ------------------------------------------------------------------ */
/* Init task (runs once, then deletes itself)                           */
/* ------------------------------------------------------------------ */
static void rtm_demo_init_task(void *arg)
{
    /* 1. WiFi */
    wifi_init();
    ESP_LOGI(TAG, "Waiting for WiFi IP...");
    xEventGroupWaitBits(s_events, WIFI_CONNECTED_BIT,
                        pdFALSE, pdTRUE, portMAX_DELAY);

    /* 2. Agora SDK init */
    int ret = agora_rtc_init(CONFIG_AGORA_APP_ID, &s_rtc_handler, NULL);
    if (ret < 0) {
        ESP_LOGE(TAG, "agora_rtc_init failed err=%d (%s)",
                 ret, agora_rtc_err_2_str(ret));
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "Agora SDK %s initialized", agora_rtc_get_version());

    /* 3. Signaling login (token=NULL disables token auth) */
    const char *token = (strlen(CONFIG_AGORA_RTM_TOKEN) > 0)
                        ? CONFIG_AGORA_RTM_TOKEN : NULL;
    ret = agora_rtc_login_rtm(CONFIG_AGORA_RTM_LOCAL_UID, token, &s_rtm_handler);
    if (ret < 0) {
        ESP_LOGE(TAG, "agora_rtc_login_rtm failed err=%d (%s)",
                 ret, agora_rtc_err_2_str(ret));
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Waiting for Signaling login...");
    xEventGroupWaitBits(s_events, RTM_LOGGED_IN_BIT,
                        pdFALSE, pdTRUE, portMAX_DELAY);

    /* 4. Start 100 Hz send task */
    ESP_LOGI(TAG, "Starting 100 Hz Signaling sender (SDK cap=60 qps — rate-limit warnings expected)");
    BaseType_t rc = xTaskCreate(rtm_send_task, "rtm_send", 4096, NULL, 5, NULL);
    configASSERT(rc == pdPASS);

    vTaskDelete(NULL);
}

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */
void rtm_demo_start(void)
{
    s_events = xEventGroupCreate();
    configASSERT(s_events);

    BaseType_t rc = xTaskCreate(rtm_demo_init_task, "rtm_init", 4096, NULL, 4, NULL);
    configASSERT(rc == pdPASS);
}
