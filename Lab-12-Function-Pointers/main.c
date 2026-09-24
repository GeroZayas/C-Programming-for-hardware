#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// ============================================================================
// Lab 12 -- Function Pointers
//
// A function pointer is just another pointer: an address in memory. The only
// twist is WHAT lives at that address -- executable code (a function) instead
// of data. You already use pointers to data (uint32_t *) and pointers to
// peripheral register blocks (a struct pointer). This lab extends the same
// idea one step further, to pointers to functions.
//
// This program is complete and runnable. Read each step, run it, watch the
// serial output, then try the challenges at the bottom.
// ============================================================================

// ---- Functions used in Step 2 (simple case) ----
int foo(int a, int b) { return a * b; }        // matches int (*)(int, int)
int bar(int a, int b) { return a + b; }         // same type -- used in a challenge

// ---- Function used in Step 3 (decoder ring) ----
int negate(int a) { return -a; }                // matches int (*)(int)

// ---- Function used in Step 4 (data-pointer argument) ----
// Accumulate an array of bytes and print the total.
void buffer_sum(uint8_t *buf, uint8_t n) {      // matches void (*)(uint8_t *, uint8_t)
  uint32_t total = 0;
  for (uint8_t i = 0; i < n; i++) total += buf[i];
  printf("    buffer_sum: total of %u bytes = %lu\n", (unsigned)n, (unsigned long)total);
}

// ---- Function used in Steps 5 and 6 (the callback template) ----
// A no-argument, no-return "application task." Here it flips a simulated LED
// state and prints it. On real hardware this would toggle a GPIO pin; QEMU
// does not model the LED, so we show the state on the serial console instead.
static int led_on = 0;
void heartbeat(void) {                          // matches void (*)(void)
  led_on = !led_on;
  printf("    heartbeat: LED %s\n", led_on ? "ON" : "OFF");
}

// ---- Function used in Step 6 (a function that TAKES a function pointer) ----
// run_task knows nothing about what it is calling. It just calls the function
// it was handed, `times` times. This is the exact mechanism behind callbacks.
void run_task(void (*cfn)(void), int times) {
  for (int i = 0; i < times; i++) {
    if (cfn) cfn();                             // guard against a null pointer
  }
}

int main(void) {
  printf("=== Lab 12: Function Pointers ===\n\n");

  // ---- Step 1: a function pointer is just a pointer (to code) ----
  // A data pointer holds the address of data; a function pointer holds the
  // address of a function. Print foo's address to make that concrete.
  printf("[1] foo lives in memory at address %p\n", (void *)foo);
  printf("    a function pointer just stores an address like that\n\n");

  // ---- Step 2: declare, assign, and call the simple case ----
  // Declaration reads: f_pointer is a pointer to a function taking (int, int)
  // and returning int.
  int (*f_pointer)(int, int);

  f_pointer = foo;        // assign: no parentheses -- we take the function, not call it
  // f_pointer = &foo;    // equivalent: the & is optional and usually omitted

  int direct  = foo(6, 7);          // call foo directly
  int via_ptr = f_pointer(6, 7);    // call through the pointer (deref is automatic)
  printf("[2] foo(6,7) = %d, f_pointer(6,7) = %d  (identical)\n\n", direct, via_ptr);

  // ---- Step 3: reading the syntax (the decoder ring) ----
  // Work the declaration middle-out: (*name) in the inner parentheses, the
  // return type on the LEFT, the argument list on the RIGHT.
  //     int (*g)(int);   ->  g points to a function taking (int), returning int
  int (*g)(int) = negate;
  printf("[3] g = negate;  g(5) = %d   (return type left, args right)\n\n", g(5));

  // ---- Step 4: a function pointer whose argument is a data pointer ----
  // Two pointer ideas stacked: a pointer to a function that itself takes a
  // pointer to data.
  void (*fp)(uint8_t *, uint8_t);
  fp = buffer_sum;
  uint8_t data[4] = {10, 20, 30, 40};
  printf("[4] calling buffer_sum through a function pointer:\n");
  fp(data, 4);
  printf("\n");

  // ---- Step 5: the callback-shaped pointer, void (*)(void) ----
  // A pointer to a function that takes no arguments and returns nothing. This
  // is the template a callback uses.
  void (*cfn)(void);
  cfn = heartbeat;
  printf("[5] calling heartbeat twice through cfn:\n");
  cfn();
  cfn();
  printf("\n");

  // ---- Step 6: passing a function pointer as an argument ----
  // run_task is generic: hand it any void(void) function and a count, and it
  // calls that function for you. It has no idea what heartbeat does.
  printf("[6] run_task(heartbeat, 3) -- generic code calling application code:\n");
  run_task(heartbeat, 3);
  printf("\n");

  printf("=== done ===\n");

  // Next lab (Lab 13) wires this same 'pass a function to be called later'
  // idea into the real SysTick driver, so the timer calls YOUR function on
  // every interrupt -- a callback.
  while (1);
  return 0;
}
