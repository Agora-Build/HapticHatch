#pragma once
#include "esp_err.h"
#include <stdint.h>

/**
 * Packet sent between devices.
 * force is normalized [0.0, 1.0].
 * timestamp_ms is used for latency diagnostics once the real SDK is wired in.
 */
typedef struct {
    uint32_t timestamp_ms;
    float    force;
} haptic_packet_t;

/** Callback invoked on the receiving side when a packet arrives. */
typedef void (*transport_rx_cb_t)(const haptic_packet_t *pkt);

/**
 * Initialize the transport layer.
 * In the loopback stub this is a no-op.
 * When the RTSA SDK is integrated, this establishes the connection.
 */
esp_err_t transport_init(void);

/**
 * Send a haptic packet to the remote device.
 * In the loopback stub this immediately calls the rx callback.
 */
esp_err_t transport_send(const haptic_packet_t *pkt);

/**
 * Register the callback to invoke when a packet is received.
 * Pass NULL to deregister.
 */
void transport_set_rx_callback(transport_rx_cb_t cb);
