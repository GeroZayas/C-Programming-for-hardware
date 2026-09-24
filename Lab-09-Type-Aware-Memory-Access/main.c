#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// --- USART1 register map (0x40013800), same based-pointer pattern as before ---
typedef volatile struct __attribute__((packed)) {
  uint32_t SR;    // 0x00  status
  uint32_t DR;    // 0x04  data
  uint32_t BRR;   // 0x08
  uint32_t CR1;   // 0x0C  control 1
  uint32_t CR2;   // 0x10
  uint32_t CR3;   // 0x14
  uint32_t GTPR;  // 0x18
} usart_t;
#define USART1 ((usart_t *)0x40013800)
#define USART_SR_RXNE (1 << 5)
#define USART_CR1_UE  (1 << 13)
#define USART_CR1_TE  (1 << 3)
#define USART_CR1_RE  (1 << 2)

int main(void) {
  printf("=== Lab 9: Type-Aware Memory Access ===\n\n");

  // ======================================================================
  // Step 1 - Promotion, and a trap it hides
  // ======================================================================
  // Safe promotion: the narrower operands are widened for the computation
  // and the result widened again for the assignment. No data is lost.
  uint8_t  a = 17;
  uint16_t b = 1000;
  uint32_t c = a + b;                 // a->uint16 for the add, result->uint32
  printf("[1] safe promotion: %u + %u = %lu\n", a, b, (unsigned long)c);

  // The trap: 1 / 1000000 is evaluated in INTEGER math (result 0) BEFORE the
  // assignment promotes it to float. Explicit casts force float division.
  float bad  = 1 / 1000000;           // integer division -> 0, then -> 0.0f
  float good = (float)1 / (float)1000000;
  printf("[1] implicit 1/1000000        = %f  (integer division bites!)\n", bad);
  printf("[1] explicit (float)1/(float)1000000 = %f\n\n", good);

  // ======================================================================
  // Step 2 - Demotion and silent data loss
  // ======================================================================
  uint32_t big  = 0xABCDCE98;
  uint16_t half = big;                // implicit demotion 32 -> 16
  uint8_t  byte = big;                // implicit demotion 32 -> 8
  printf("[2] 32-bit value : 0x%08lX\n", (unsigned long)big);
  printf("[2] into uint16_t: 0x%04X      (upper 16 bits discarded)\n", (unsigned)half);
  printf("[2] into uint8_t : 0x%02X        (only the low byte kept)\n", (unsigned)byte);
  uint8_t byte2 = (uint8_t)big;       // explicit cast: same result, clear intent
  printf("[2] explicit (uint8_t)big = 0x%02X  (documents that the loss is intended)\n\n",
         (unsigned)byte2);

  // ======================================================================
  // Step 3 - Type-aware register access: read a byte from the USART
  // ======================================================================
  // Enable the USART and its receiver so typed bytes land in DR / set RXNE.
  USART1->CR1 |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);

  printf("[3] Type one character, then press Enter:\n");

  // The serial terminal is line-buffered (canonical mode), so your character
  // is delivered only after you press Enter, which also sends a newline.
  // Read bytes until we get a real character, skipping the carriage-return /
  // newline that the Enter key produces. Each read of DR is a 32-bit access.
  uint32_t raw;
  char ch;
  do {
    while (!(USART1->SR & USART_SR_RXNE)) { }   // wait for a received byte
    raw = USART1->DR;
    ch  = (char)(raw & 0xFF);
  } while (ch == '\n' || ch == '\r');           // ignore the Enter key press

  // Now extract the 8 data bits four different ways (the four lines from the
  // video's slide), all applied to the single byte we captured.
  char e1 = raw;                         // implicit demotion 32 -> 8
  char e2 = (char)raw;                   // explicit demotion
  char e3 = raw & 0x000000FF;            // mask + implicit demotion
  char e4 = (char)(raw & 0x000000FF);    // most explicit (best practice)

  printf("[3] raw 32-bit DR read      = 0x%08lX\n", (unsigned long)raw);
  printf("[3] implicit demotion       : 0x%02X '%c'\n", (unsigned char)e1, e1);
  printf("[3] explicit cast           : 0x%02X '%c'\n", (unsigned char)e2, e2);
  printf("[3] mask + implicit demotion: 0x%02X '%c'\n", (unsigned char)e3, e3);
  printf("[3] mask + explicit cast    : 0x%02X '%c'\n", (unsigned char)e4, e4);

  // On this emulator the upper bits of DR read as 0. On real silicon the
  // reserved bits could hold anything, so the mask is what keeps you safe.
  uint32_t junk = 0xFFFFFF00u | (raw & 0xFFu);   // pretend the reserved bits held junk
  printf("[3] if reserved bits held junk: 0x%08lX -> masked byte 0x%02X (mask rescues it)\n\n",
         (unsigned long)junk, (unsigned)(junk & 0xFFu));

  // ======================================================================
  // Step 4 - Watch your alignment (ADC-style example)
  // ======================================================================
  // An 8-bit result LEFT-aligned in a 16-bit field: the byte you want is in
  // the high half, so a plain demotion grabs the wrong (zero) low byte.
  uint32_t adc_left = 0x00009A00;
  uint8_t  wrong    = adc_left;             // takes low byte -> 0x00 (wrong!)
  uint8_t  right    = (adc_left >> 8);      // shift first, then demote -> 0x9A
  printf("[4] left-aligned result 0x%04lX: naive=0x%02X (wrong), shifted=0x%02X (correct)\n",
         (unsigned long)adc_left, wrong, right);

  printf("\n=== done ===\n");
  while (1) { }        // idle; on hardware main never returns
  return 0;
}
