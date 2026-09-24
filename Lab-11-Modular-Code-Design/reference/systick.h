#pragma once

#include <stdint.h>

// Initialize SysTick for 1 kHz interrupts (configures, but does not start).
int systick_init(void);

// Start and stop the SysTick timer.
int systick_start(void);
int systick_stop(void);

// Get the current tock count (written through the pointer argument).
int systick_get_tocks(uint32_t *tocks);
