// ============================================================================
// Lab 7 - Version 3 (FIX #2): the "critical section" technique.
//
// This is a REFERENCE file. It is NOT compiled by the Makefile. To try it,
// copy its contents over main.c (back up the original first), or paste the
// changed pieces into main.c as the README's Version 3 step describes.
//
// Idea: keep the shared 'time' struct and the time-keeping ISR exactly as in
// Version 1, but make main's read of the pair UNINTERRUPTIBLE. We briefly
// disable interrupts, grab both fields, and re-enable. The ISR cannot fire
// between the two reads, so they always come from the same instant.
//
// Trade-off: this blocks ALL interrupts for the duration, so the critical
// section must be as SHORT as possible. Notice we do the (slow) printf AFTER
// re-enabling interrupts, never inside the critical section.
// ============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

// Cortex-M: 'cpsid i' masks interrupts, 'cpsie i' unmasks them. These are the
// disable_interrupts()/enable_interrupts() the video asks you to assume exist.
static inline void disable_interrupts(void) { __asm volatile ("cpsid i" ::: "memory"); }
static inline void enable_interrupts(void)  { __asm volatile ("cpsie i" ::: "memory"); }

// Same shared clock and same ISR as Version 1 -- unchanged.
volatile struct {
  uint16_t msec;
  uint32_t sec;
} time;

void SysTick_Handler(void) {
  time.msec++;
  if (time.msec >= 1000) {
    time.msec = 0;
    time.sec++;
  }
}

// Paces the output so it is readable. Kept OUTSIDE the critical section.
static void pace(void) {
  for (volatile uint32_t i = 0; i < 16000000; i++) { }
}

int main(void) {
  printf("Hello, World!\n");

  time.msec = 0;
  time.sec  = 0;

  SYSTICK->ctrl.clksource = 1;
  SYSTICK->ctrl.tick_int  = 1;
  SYSTICK->val  = 8000;
  SYSTICK->load = 8000;
  SYSTICK->ctrl.enable = 1;

  while (1) {
    uint16_t m;
    uint32_t s;

    // ---- CRITICAL SECTION: read both fields atomically w.r.t. the ISR ----
    disable_interrupts();
    m = time.msec;
    s = time.sec;
    enable_interrupts();
    // ----------------------------------------------------------------------

    pace();  // widen the loop like Version 1 did -- but the tear is gone,
             // because the two reads above could not be interrupted.
    printf("timer: %lu.%03u\n", (unsigned long)s, (unsigned)m);
  }

  return 0;
}
