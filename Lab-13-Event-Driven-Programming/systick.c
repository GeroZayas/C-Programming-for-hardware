#include "systick.h"

#include <stdlib.h>
#include <stdio.h>

// --- SysTick register map (kept HERE, not in the header) ---
// Hiding the memory map inside the module stops other .c files from reaching
// around our interface to poke the hardware directly.
typedef volatile struct __attribute__((packed)) {
  struct {
    uint32_t enable     : 1;
    uint32_t tick_int   : 1;
    uint32_t clksource  : 1;
    uint32_t reserved   : 13;
    uint32_t count_flag : 1;
  } ctrl;
  uint32_t load;
  uint32_t val;
  uint32_t calib;
} systick_t;

#define SYSTICK_BASE (0xE000E010)
#define SYSTICK ((systick_t *)SYSTICK_BASE)

// Module-level shared variable. `static` hides it from other .c files but
// leaves it visible to everything inside this module.
static uint32_t systick_tocks = 0;

void SysTick_Handler(void) {
  static int ticks = 0;
  ticks++;
  if (ticks == 1000) {
    systick_tocks++;
    // NOTE: this printf is application-specific -- it does not really belong
    // in a generic driver. Moving it out cleanly needs function pointers
    // (Lab 12) and a callback (Lab 13). For now it stays here.
    printf("tock! (#%ld)\n", systick_tocks);
    ticks = 0;
  }
}

// Initialize SysTick for 1 kHz interrupts (configures, but does not start).
int systick_init(void) {
  if (SYSTICK->ctrl.enable == 1) {
    return -1;                   // don't reconfigure a running timer
  }
  SYSTICK->ctrl.clksource = 1;   // use the 8 MHz clock
  SYSTICK->ctrl.tick_int  = 1;   // generate interrupts
  SYSTICK->val  = 8000;
  SYSTICK->load = 8000;          // 8 MHz / 8000 = 1 kHz
  return 0;
}

// Start and stop the SysTick timer.
int systick_start(void) {
  SYSTICK->ctrl.enable = 1;
  return 0;
}
int systick_stop(void) {
  SYSTICK->ctrl.enable = 0;
  return 0;
}

// Get the current tock count (written through the pointer argument).
int systick_get_tocks(uint32_t *tocks) {
  if (tocks == 0) {
    return -1;                   // NULL pointer guard (defensive coding)
  }
  *tocks = systick_tocks;
  return 0;
}
