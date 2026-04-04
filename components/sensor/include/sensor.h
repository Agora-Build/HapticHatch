#pragma once
#include "esp_err.h"

/**
 * Initialize the force sensor.
 * Must be called before sensor_read().
 */
esp_err_t sensor_init(void);

/**
 * Read the current force value.
 * Returns a normalized value in [0.0, 1.0].
 * 0.0 = no force, 1.0 = maximum force.
 */
float sensor_read(void);
