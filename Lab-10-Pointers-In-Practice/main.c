#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// --- USART1 register map (for the Step 6 capstone) ---
typedef volatile struct __attribute__((packed)) {
  uint32_t SR, DR, BRR, CR1, CR2, CR3, GTPR;
} usart_t;
#define USART1 ((usart_t *)0x40013800)
#define USART_SR_RXNE (1 << 5)
#define USART_CR1_UE  (1 << 13)
#define USART_CR1_TE  (1 << 3)
#define USART_CR1_RE  (1 << 2)

typedef int err_t;   // defensive-coding convention: 0 = ok, non-zero = error

// ===== Step 1: pass by value vs pass by reference =====
int inc_value(int x)  { return ++x; }     // operates on a local copy
int inc_ref(int *x)   { return ++*x; }    // operates through the caller's address

// ===== Step 2: arrays decay to pointers =====
void fill_array(int arg[], int n) {
  // Inside a function, an array parameter is really a pointer, so sizeof(arg)
  // is the size of a pointer (4), NOT the array. (The compiler even warns.)
  printf("    inside fill_array: sizeof(arg) = %u  (a pointer, not the array)\n",
         (unsigned)sizeof(arg));
  for (int i = 0; i < n; i++) arg[i] = i * 10;
}

// ===== Step 3: structs are passed by value (a deep copy) =====
typedef struct { int x; int y; } v2_t;
int bump_copy(v2_t s)  { s.x++; s.y++; return s.x + s.y; }   // copy: caller unchanged
int bump_ref(v2_t *s)  { s->x++; s->y++; return s->x + s->y; }

// ===== Step 4: defensive function programming =====
err_t add_checked(int a, int b, int *out) {
  if (a < 0 || b > 100) return -1;   // reject bad arguments
  *out = a + b;                      // return the value through a pointer
  return 0;                          // success
}

// ===== Step 5: walk a buffer two ways =====
void inc_buf_index(uint8_t buf[], uint8_t n) { for (uint8_t k = 0; k < n; k++) buf[k] += 1; }
void inc_buf_ptr(uint8_t *buf, uint8_t n)    { for (uint8_t k = 0; k < n; k++) *(buf++) += 1; }

// ===== Step 6 capstone: assemble received bytes into 32-bit words =====
#define BUFLEN 2
static uint32_t rxbuf[BUFLEN];
void process_data(uint32_t *data) {
  printf("[6] process_data got %d assembled word(s):\n", BUFLEN);
  for (int i = 0; i < BUFLEN; i++)
    printf("      buf[%d] = 0x%08lX\n", i, (unsigned long)data[i]);
}

int main(void) {
  printf("=== Lab 10: Pointers in Practice ===\n\n");

  // ---- Step 1 ----
  int a = 1;
  int r1 = inc_value(a);
  printf("[1] inc_value(a): a = %d, returned %d   (a unchanged: pass by value)\n", a, r1);
  int r2 = inc_ref(&a);
  printf("[1] inc_ref(&a) : a = %d, returned %d   (a changed: pass by reference)\n\n", a, r2);

  // ---- Step 2 ----
  int nums[4];
  printf("[2] in main: sizeof(nums) = %u  (the whole array)\n", (unsigned)sizeof(nums));
  fill_array(nums, 4);
  printf("[2] after fill_array: nums = {%d,%d,%d,%d}  (caller's array changed: arrays decay to pointers)\n\n",
         nums[0], nums[1], nums[2], nums[3]);

  // ---- Step 3 ----
  v2_t s = {0, 1};
  int rc = bump_copy(s);
  printf("[3] bump_copy(s): s = {%d,%d}, returned %d   (s unchanged: struct copied by value)\n",
         s.x, s.y, rc);
  int rr = bump_ref(&s);
  printf("[3] bump_ref(&s): s = {%d,%d}, returned %d   (s changed: passed by reference)\n\n",
         s.x, s.y, rr);

  // ---- Step 4 ----
  int out;
  if (add_checked(1, 2, &out)) printf("[4] add_checked(1, 2): error\n");
  else                         printf("[4] add_checked(1, 2): ok, out = %d\n", out);
  if (add_checked(-1, 2, &out)) printf("[4] add_checked(-1, 2): error (bad argument, out untouched)\n\n");
  else                          printf("[4] add_checked(-1, 2): ok, out = %d\n\n", out);

  // ---- Step 5 ----
  uint8_t buf[4] = {0, 1, 2, 3};
  printf("[5] buffer start         : {%d,%d,%d,%d}\n", buf[0], buf[1], buf[2], buf[3]);
  inc_buf_index(buf, 4);
  printf("[5] after array indexing : {%d,%d,%d,%d}\n", buf[0], buf[1], buf[2], buf[3]);
  inc_buf_ptr(buf, 4);
  printf("[5] after pointer walk   : {%d,%d,%d,%d}  (both do the same thing)\n\n",
         buf[0], buf[1], buf[2], buf[3]);

  // ---- Step 6 (capstone) ----
  // Data arrives one BYTE at a time but we want 32-bit WORDS. Alias the word
  // buffer as bytes with a uint8_t pointer and store each received byte in turn.
  USART1->CR1 |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);
  printf("[6] Type %d characters, then press Enter (they assemble into %d word(s)):\n",
         BUFLEN * 4, BUFLEN);

  uint8_t *pbuf = (uint8_t *)rxbuf;   // the same buffer, viewed as bytes
  int index = 0;
  while (index < BUFLEN * 4) {
    while (!(USART1->SR & USART_SR_RXNE)) { }
    uint32_t raw = USART1->DR;
    char ch = (char)(raw & 0xFF);
    if (ch == '\n' || ch == '\r') continue;    // ignore the Enter key
    pbuf[index++] = (uint8_t)(raw & 0xFF);      // store the byte through the alias
  }
  process_data(rxbuf);   // the bytes are now readable as 32-bit words

  printf("\n=== done ===\n");
  while (1) { }
  return 0;
}
