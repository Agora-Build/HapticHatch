#pragma once

/**
 * Initialize WiFi + Agora RTM and start a 100 Hz send task.
 *
 * Non-blocking: spawns an init task that handles the WiFi association and
 * RTM login sequence before starting the sender.  app_main() can call this
 * and continue creating other tasks without waiting.
 */
void rtm_demo_start(void);
