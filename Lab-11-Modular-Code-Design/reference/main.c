#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "systick.h"   // our new module's public interface

// ============================================================================
// Lab 11 FINISHED main.c.
//
// All of the SysTick register map, the global counter, and the interrupt
// handler have moved into systick.c. What is left here is pure application
// intent: start the timer and let it run. Notice how readable main became.
// ============================================================================

int main(void) {
  printf("Hello, World!\n");

  systick_init();    // configure SysTick for 1 kHz interrupts
  systick_start();   // start counting

  // Event-driven: the "tock!" message now comes from inside the module.
  while (1);

  return 0;
}
