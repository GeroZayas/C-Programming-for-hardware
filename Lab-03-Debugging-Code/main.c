#include <stdio.h>
#include <stdlib.h>

#include "sleep.h"

int main(void) {
  // Create some variables to use in counter;
  unsigned int counter = 0;
  unsigned int counter_increment = 1;

  // Loop forever
  while(1) {
    sleep(&counter, &counter_increment);
  }

  // This is necessary to keep the compiler happy (main always returns int)
  return counter;
}