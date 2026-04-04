#pragma once
#include "esp_err.h"

/**
 * Initialize the haptic actuator.
 * Must be called before haptic_play().
 */
esp_err_t haptic_init(void);

/**
 * Play a haptic effect at the given intensity.
 * intensity: normalized value in [0.0, 1.0].
 * 0.0 = no effect, 1.0 = maximum intensity.
 * Values outside [0.0, 1.0] are clamped by the implementation.
 */
void haptic_play(float intensity);
