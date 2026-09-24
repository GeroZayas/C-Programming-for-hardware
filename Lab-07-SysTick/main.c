#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// ============================================================================
// SysTick register map (a memory-mapped peripheral).
// Same 'volatile packed struct + based pointer' pattern you built in Lab 6.
// ============================================================================
typedef volatile struct __attribute__((packed)) {
  struct {
    uint32_t enable    : 1;
    uint32_t tick_int  : 1;   // <-- NEW vs Lab 6: request an interrupt on reload
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

// ============================================================================
// Critical-section helpers (used in Version 3 of this lab).
// On Cortex-M, 'cpsid i' masks interrupts and 'cpsie i' unmasks them.
// These stand in for the disable_interrupts()/enable_interrupts() the video
// asks you to "assume exist." They are unused until Version 3.
// ============================================================================
static inline void disable_interrupts(void) { __asm volatile ("cpsid i" ::: "memory"); }
static inline void enable_interrupts(void)  { __asm volatile ("cpsie i" ::: "memory"); }

// ============================================================================
// Shared state: written by the ISR (one "thread") and read by main (another).
// This is the ONLY data both threads touch, so it is where a race can occur.
// It must be volatile: the ISR changes it behind the compiler's back.
// ============================================================================
volatile struct {
  uint16_t msec;
  uint32_t sec;
} time;

// The SysTick interrupt fires at 1 kHz (once per millisecond). This handler
// keeps a running clock in 'time'. It replaces the weak SysTick_Handler in
// the startup file simply by having the same name.
void SysTick_Handler(void) {
  time.msec++;
  if (time.msec >= 1000) {
    time.msec = 0;
    time.sec++;
  }
}

// Deliberately waste a little time. On real hardware you would NEVER add this.
// Here it widens the gap between main's two reads of 'time' below, so the race
// condition becomes visible on (almost) every run instead of once in a billion.
static void widen_window(void) {
  for (volatile uint32_t i = 0; i < 16000000; i++) { }
}

int main(void) {
  printf("Hello, World!\n");

  time.msec = 0;
  time.sec  = 0;

  // Configure SysTick for 1 kHz interrupts: 8 MHz / 8000 = 1 kHz.
  SYSTICK->ctrl.clksource = 1;  // use the 8 MHz processor clock
  SYSTICK->ctrl.tick_int  = 1;  // generate an interrupt each time it reloads
  SYSTICK->val  = 8000;
  SYSTICK->load = 8000;
  SYSTICK->ctrl.enable = 1;     // start it: the ISR now runs in the background

  // main's job: print the current time. Think of each pass as a "button press"
  // asking "what time is it?" -- exactly the scenario from the video.
  while (1) {
    // ---- THE BUG (Version 1) --------------------------------------------
    // We read the shared clock in TWO separate steps. Between them the ISR
    // can fire and roll the clock over (msec 999 -> 0, sec += 1). When it
    // does, our two halves come from different instants: a "torn read."
    uint16_t m = time.msec;   // read milliseconds first...
    widen_window();           // ...an interrupt can land in this gap...
    uint32_t s = time.sec;    // ...now read seconds (maybe already bumped!).

    // Watch the output: most lines are fine, but around each 1-second
    // rollover you will see an impossible jump such as  timer: 4.999
    // (real time was 3.999 -- 'sec' updated after we captured msec=999).
    printf("timer: %lu.%03u\n", (unsigned long)s, (unsigned)m);
  }

  return 0;
}
