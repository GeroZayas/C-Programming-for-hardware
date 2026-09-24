#include <stdio.h>
#include <stdlib.h>

int main(void) {
  // Say Hello
  printf("Hello, World!\n");
  
  // When running direcly on hardware, the processor always 
  // needs explicit instructions on what to do next.
  // So we never end the main function, instead we just loop forever!
  while(1);

  // This is necessary to keep the compiler happy (main always returns int)
  return 0;
}