#include <stdio.h>
#include <stdlib.h>

#include "systick.h"

int main(void) {
  // Say Hello
  printf("Hello, World!\n");

  // Configure the SysTick hardware timer through its structured register map.
  // Note: no interrupts here. We poll the count_flag ourselves in the loop below.
  SYSTICK->ctrl.clksource = 1; // Use the 8 MHz processor clock
  SYSTICK->load = 8000;        // Reload value: counts 8000 down to 0
  SYSTICK->val  = 8000;        // Start the counter at this value
  SYSTICK->ctrl.enable = 1;    // Start the timer!

  // Poll count_flag: the hardware sets it when the counter reaches 0, and
  // reading it clears it. This ONLY works because the register map is declared
  // 'volatile', which forces the compiler to re-read the value from hardware on
  // every pass through the loop instead of caching it in a register.
  while (1) {
    while ( !SYSTICK->ctrl.count_flag );  // wait until the timer reaches 0
    printf("Tock!\n");
  }

  // When running directly on hardware the processor must always have something
  // to do, so main never returns. (Unreachable here, but keeps the compiler happy.)
  return 0;
}
