#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "systick.h"

void systick_cbfn(void) {
  uint32_t systick_tocks;
  systick_get_tocks(&systick_tocks);
  printf("tock! (#%ld)\n", systick_tocks);
}

int main(void) {
  // Say Hello
  printf("Hello, World!\n");

  systick_init(systick_cbfn);
  systick_start();

  // When running direcly on hardware, the processor always 
  // needs explicit instructions on what to do next.
  // So we never end the main function, instead we just loop forever!
  while(1);

  // This is necessary to keep the compiler happy (main always returns int)
  return 0;
}