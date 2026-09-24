#pragma once 

#include <stdint.h>

// Initialize systick for 8 kHz interrupts
int systick_init(void(*cbfn)(void));

// Start and stop the systick timer
int systick_start(void);
int systick_stop(void);

// Get current tock count
int systick_get_tocks(uint32_t *tocks);
