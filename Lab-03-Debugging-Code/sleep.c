#include "sleep.h"

void sleep(unsigned int *pcounter, unsigned int *pcounter_increment) {
    *pcounter = *pcounter + *pcounter_increment;
}