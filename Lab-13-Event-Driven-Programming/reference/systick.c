#include "systick.h"

#include <stdlib.h>
#include <stdio.h>

// Systick Memory Map
typedef volatile struct __attribute__((packed)) {
  struct {
    uint32_t enable : 1;
    uint32_t tick_int : 1;
    uint32_t clksource : 1;
    uint32_t reserved : 13;
    uint32_t count_flag : 1;
  } ctrl;
  uint32_t load;
  uint32_t val;
  uint32_t calib;
} systick_t;

#define SYSTICK_BASE (0xE000E010)
#define SYSTICK ((systick_t *)SYSTICK_BASE)

// Module shared variable, increments every 1000 systick ticks
static uint32_t systick_tocks = 0;

static void(*systick_cbfn)(void);

void SysTick_Handler(void) {
  static int ticks = 0;
  // increment ticks every interrupt, 
  ticks++;
  // increment tocks every 1000 interrupts and 
  // call a "tock" function
  if( ticks == 1000 ) {
    systick_tocks++;
    if( systick_cbfn ) {
      systick_cbfn();
    }
    ticks = 0;
  }
}

// Initialize systick for 8 kHz interrupts
int systick_init(void(*cbfn)(void)) {
    systick_cbfn = cbfn;
    if( SYSTICK->ctrl.enable == 1 ) {
        return -1;
    }

    // Initialize SysTick for 1kHz interrupts
  SYSTICK->ctrl.clksource = 1; // use 8 MHz clock
  SYSTICK->ctrl.tick_int = 1;  // generate interrupts
  SYSTICK->val = 8000;         // 8 MHz / 8000 = 1kHz
  SYSTICK->load = 8000;
  return 0;
}

// Start and stop the systick timer
int systick_start(void) {
    SYSTICK->ctrl.enable = 1;    // start the counter
    return 0;
}
int systick_stop(void) {
    SYSTICK->ctrl.enable = 0;
    return 0;
}

// Get current tock count
int systick_get_tocks(uint32_t *tocks) {
    if( tocks == 0 ) 
        return -1;
    *tocks = systick_tocks;
    return 0;
}