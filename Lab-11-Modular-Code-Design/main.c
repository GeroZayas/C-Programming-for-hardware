#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// ============================================================================
// Lab 11 STARTING POINT -- one big "monolithic" main.c.
//
// Everything lives in this single file: the SysTick register map, a global
// counter, the interrupt handler, and the application's "tock!" message.
// It works, but two different ideas are tangled together here:
//   1. a SysTick timer *driver*   (the register map, init, the handler)
//   2. this program's *application* (printing "tock!" once per second)
//
// Over this lab you will refactor these apart into a reusable systick module
// (systick.h / systick.c), leaving main.c holding only application logic.
// ============================================================================

// --- SysTick register map ---
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

// Global shared variable, incremented every 1000 SysTick ticks.
// (Global for now -- we will hide it inside the module later in this lab.)
uint32_t systick_tocks = 0;

// Called automatically every time the SysTick interrupt fires.
// Takes no arguments and returns nothing -- it is invoked from an arbitrary
// point in the program, so it can neither receive nor return values.
void SysTick_Handler(void) {
  static int ticks = 0;   // keeps its value between interrupts
  ticks++;
  if (ticks == 1000) {
    systick_tocks++;
    printf("tock! (#%ld)\n", systick_tocks);   // <-- application code, stuck inside the driver
    ticks = 0;
  }
}

int main(void) {
  // Banner: confirms our code is running and the serial terminal works.
  printf("Hello, World!\n");

  // Initialize SysTick for 1 kHz interrupts.
  SYSTICK->ctrl.clksource = 1;   // use the 8 MHz clock
  SYSTICK->ctrl.tick_int  = 1;   // generate interrupts
  SYSTICK->val  = 8000;
  SYSTICK->load = 8000;          // 8 MHz / 8000 = 1 kHz
  SYSTICK->ctrl.enable = 1;      // start the counter

  // Event-driven: nothing to do in the main loop; just wait for interrupts.
  while (1);

  return 0;   // never reached (keeps the compiler happy)
}
