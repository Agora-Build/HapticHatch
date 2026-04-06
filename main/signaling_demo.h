#pragma once

/**
 * Initialize WiFi + Agora Signaling and start a 100 Hz send task.
 *
 * Non-blocking: spawns an init task that handles the WiFi association and
 * Signaling login sequence before starting the sender.  app_main() can call
 * this and continue creating other tasks without waiting.
 */
void signaling_demo_start(void);
