#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// ============================================================================
// Register maps (the based-pointer pattern from Lab 6, now for the peripherals
// the Module 3 videos use). Each is a volatile packed struct laid over the
// peripheral's physical base address.
// ============================================================================

// --- RCC: reset and clock control (0x40021000) ---
typedef volatile struct __attribute__((packed)) {
  uint32_t CR;        // 0x00
  uint32_t CFGR;      // 0x04
  uint32_t CIR;       // 0x08
  uint32_t APB2RSTR;  // 0x0C
  uint32_t APB1RSTR;  // 0x10
  uint32_t AHBENR;    // 0x14
  uint32_t APB2ENR;   // 0x18
  uint32_t APB1ENR;   // 0x1C
} rcc_t;
#define RCC ((rcc_t *)0x40021000)
#define RCC_APB2ENR_IOPAEN    (1 << 2)   // GPIOA clock enable
#define RCC_APB2ENR_USART1EN  (1 << 14)  // USART1 clock enable

// --- USART1: serial peripheral (0x40013800) ---
typedef volatile struct __attribute__((packed)) {
  uint32_t SR;    // 0x00  status
  uint32_t DR;    // 0x04  data
  uint32_t BRR;   // 0x08  baud rate
  uint32_t CR1;   // 0x0C  control 1
  uint32_t CR2;   // 0x10
  uint32_t CR3;   // 0x14
  uint32_t GTPR;  // 0x18
} usart_t;
#define USART1 ((usart_t *)0x40013800)
#define USART_SR_RXNE  (1 << 5)   // receive register not empty
#define USART_CR1_UE   (1 << 13)  // USART enable
#define USART_CR1_TE   (1 << 3)   // transmit enable
#define USART_CR1_RE   (1 << 2)   // receive enable

// Simple busy-wait so the toggle loop is slow enough to read.
static void delay(volatile uint32_t n) { while (n--) { __asm volatile("nop"); } }

int main(void) {
  printf("Hello, World!\n");

  // ---- SET (|=): turn on the peripheral clocks. Real silicon requires this
  //      before GPIOA or USART1 respond. It is another "setting bits" example.
  //      (The Virtual Lab's QEMU models the USART registers but not the clock
  //      tree, so this register reads back 0 here. See the README note.) ----
  RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN);
  printf("RCC->APB2ENR      = 0x%08lX  (unmodeled in QEMU; needed on real HW)\n",
         (unsigned long)RCC->APB2ENR);

  // ---- SET (|=): enable the USART and both its transmitter and receiver ----
  printf("USART1->CR1 (pre) = 0x%08lX\n", (unsigned long)USART1->CR1);
  USART1->CR1 |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);
  printf("USART1->CR1 (set) = 0x%08lX  <- UE|TE|RE set\n", (unsigned long)USART1->CR1);

  // ---- MASK (&): check the RXNE flag to see if a byte has arrived ----
  if (USART1->SR & USART_SR_RXNE)
    printf("RXNE is set: a byte is waiting\n");
  else
    printf("RXNE is clear: no data yet (SR = 0x%08lX)\n", (unsigned long)USART1->SR);

  // ---- CLEAR (&= ~): turn the receiver back off (bit 2). We clear RE, not TE,
  //      so the printf transmit path keeps working. ----
  USART1->CR1 &= ~USART_CR1_RE;
  printf("USART1->CR1 (clr) = 0x%08lX  <- RE cleared\n", (unsigned long)USART1->CR1);

  // ---- TOGGLE (^=): the same XOR that blinks an LED on real hardware,
  //      "GPIOA->ODR ^= (1 << 4);", applied here to a register the Virtual Lab
  //      can show you live: we flip the receive-enable bit on and off and watch
  //      CR1.RE blink in the Peripherals view. ----
  int count = 0;
  while (1) {
    USART1->CR1 ^= USART_CR1_RE;
    printf("toggle %d: CR1 = 0x%08lX  (RE %s)\n", ++count,
           (unsigned long)USART1->CR1,
           (USART1->CR1 & USART_CR1_RE) ? "on" : "off");
    delay(4000000);
  }
  return 0;
}
