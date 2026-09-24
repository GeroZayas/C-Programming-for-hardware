// ============================================================================
// Lab 7 - Version 2 (FIX #1): the "single-setter flag" technique.
//
// This is a REFERENCE file. It is NOT compiled by the Makefile. To try it,
// copy its contents over main.c (back up the original first), or paste the
// changed pieces into main.c as the README's Version 2 step describes.
//
// Idea: the ISR no longer touches the shared time. It only SETS a flag
// (a single 32-bit store -- an atomic, indivisible operation). main OWNS all
// of the time arithmetic in its own thread, so the two threads never share a
// mutable number, and the race disappears.
// ============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef volatile struct __attribute__((packed)) {
  struct {
    uint32_t enable    : 1;
    uint32_t tick_int  : 1;
    uint32_t clksource : 1;
    uint32_t reserved  : 13;
    uint32_t count_flag : 1;
  } ctrl;
  uint32_t load;
  uint32_t val;
  uint32_t calib;
} systick_t;

#define SYSTICK_BASE (0xE000E010)
#define SYSTICK ((systick_t *)SYSTICK_BASE)

// The ONLY thing shared between the ISR and main. Setting a bool is a single
// store instruction, so it is atomic: main can never catch it "half written."
volatile bool systick_flag = false;

// The interrupt just raises the flag. It owns no time state at all.
void SysTick_Handler(void) {
  systick_flag = true;   // atomic set -- nothing shared is read-modified-written
}

int main(void) {
  printf("Hello, World!\n");

  // Time now lives in MAIN's thread only. The ISR never touches these.
  uint16_t msec = 0;
  uint32_t sec  = 0;

  SYSTICK->ctrl.clksource = 1;
  SYSTICK->ctrl.tick_int  = 1;
  SYSTICK->val  = 8000;
  SYSTICK->load = 8000;
  SYSTICK->ctrl.enable = 1;

  while (1) {
    // Process each tick as soon as the flag says one happened. This must stay
    // fast: with a single-setter bool we count "an event occurred," not "how
    // many," so main has to keep up with the 1 kHz tick or it loses time.
    if (systick_flag) {
      systick_flag = false;   // atomic clear
      msec++;
      if (msec >= 1000) {
        msec = 0;
        sec++;
      }
      // Because ONLY main touches msec/sec, reading them here is always
      // consistent -- there is no other thread to interleave with.
      // Print 4x/second so the output is readable.
      if (msec % 250 == 0) {
        printf("timer: %lu.%03u\n", (unsigned long)sec, (unsigned)msec);
      }
    }
  }

  return 0;
}
