#include "myfunc.h"

const float pi = 3.14159;

float myfunc( int *fooptr, int *barptr ) {
  float result = (*fooptr + *barptr) * pi;
  return result;
}