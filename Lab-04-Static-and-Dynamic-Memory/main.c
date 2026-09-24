#include <stdio.h>
#include <stdlib.h>

#include "myfunc.h"

int main(void) {
  static int bar = 1;

  float ans;
  for( int k = 0; k < 3; k++ ) {
    int foo = k;
    ans += myfunc( &foo, &bar );
  }

  // When running direcly on hardware, the processor always 
  // needs explicit instructions on what to do next.
  // So we never end the main function, instead we just loop forever!
  while(1);

  // This is necessary to keep the compiler happy (main always returns int)
  return (int)ans;
}