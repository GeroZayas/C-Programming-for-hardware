#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>   // offsetof

/* =====================================================================
 * Lab 5 - How C data types map to memory.
 * We declare native types, a struct (padded vs packed), a union, and a
 * bitfield, then print their sizes/offsets/addresses. Use the printed
 * addresses with the Memory view to see the actual bytes.
 * ===================================================================== */

/* ---------- Part B: struct layout (padded vs packed) ---------- */
/* Same members in the same order; the only difference is 'packed'.   */
struct demo {
    int   a, b;     /* 4 + 4 */
    short c;        /* 2     */
    int   d;        /* 4     */
    char  e;        /* 1     */
    short f, g;     /* 2 + 2 */
};                  /* sum of members = 19 bytes */

struct __attribute__((packed)) demo_packed {
    int   a, b;
    short c;
    int   d;
    char  e;
    short f, g;
};

/* ---------- Part C: union aliasing ---------- */
/* The same bytes, viewed as 8-bit chars or as 16-bit shorts.         */
union overlay {
    char  a[5];     /* 8-bit view  */
    short b[3];     /* 16-bit view */
};

/* ---------- Part D: bitfield over a register-shaped word ---------- */
/* Models the SysTick control register. Reading '.raw' shows the      */
/* underlying 32-bit value, exactly how we'll touch hardware later.   */
union systick_ctrl {
    struct {
        unsigned int enable     : 1;   /* bit 0      */
        unsigned int tick_int   : 1;   /* bit 1      */
        unsigned int clksource  : 1;   /* bit 2      */
        unsigned int reserved   : 13;  /* bits 3..15 */
        unsigned int count_flag : 1;   /* bit 16     */
    } bits;
    unsigned int raw;
};

/* Global instances: fixed addresses that are easy to find in the Memory view.
 * Members are set to recognizable byte patterns so padding gaps stand out.   */
struct demo        g_demo        = { 0x11111111, 0x22222222, 0x3333, 0x44444444, 0x55, 0x6666, 0x7777 };
struct demo_packed g_demo_packed = { 0x11111111, 0x22222222, 0x3333, 0x44444444, 0x55, 0x6666, 0x7777 };
union  overlay     g_overlay;
union  systick_ctrl g_ctrl;

int main(void) {

    /* ---- Part A: native type sizes and alignment ---- */
    char  c_var = 'A';
    short s_var = 0x1234;
    int   i_var = 0x12345678;
    float f_var = 3.14159f;
    bool  b_var = true;

    printf("== Part A: native type sizes ==\n");
    printf("sizeof(char)  = %u\n", (unsigned)sizeof(char));
    printf("sizeof(short) = %u\n", (unsigned)sizeof(short));
    printf("sizeof(int)   = %u\n", (unsigned)sizeof(int));
    printf("sizeof(float) = %u\n", (unsigned)sizeof(float));
    printf("sizeof(bool)  = %u\n", (unsigned)sizeof(bool));
    printf("addresses: c=%p s=%p i=%p f=%p b=%p\n",
           (void*)&c_var, (void*)&s_var, (void*)&i_var, (void*)&f_var, (void*)&b_var);

    /* ---- Part B: struct padding vs packed ---- */
    printf("\n== Part B: struct layout ==\n");
    printf("sizeof(struct demo)        = %u  (members sum to 19)\n", (unsigned)sizeof(struct demo));
    printf("sizeof(struct demo_packed) = %u\n", (unsigned)sizeof(struct demo_packed));
    printf("demo   offsets: a=%u b=%u c=%u d=%u e=%u f=%u g=%u\n",
           (unsigned)offsetof(struct demo, a), (unsigned)offsetof(struct demo, b),
           (unsigned)offsetof(struct demo, c), (unsigned)offsetof(struct demo, d),
           (unsigned)offsetof(struct demo, e), (unsigned)offsetof(struct demo, f),
           (unsigned)offsetof(struct demo, g));
    printf("packed offsets: a=%u b=%u c=%u d=%u e=%u f=%u g=%u\n",
           (unsigned)offsetof(struct demo_packed, a), (unsigned)offsetof(struct demo_packed, b),
           (unsigned)offsetof(struct demo_packed, c), (unsigned)offsetof(struct demo_packed, d),
           (unsigned)offsetof(struct demo_packed, e), (unsigned)offsetof(struct demo_packed, f),
           (unsigned)offsetof(struct demo_packed, g));
    printf("inspect in Memory view: &g_demo=%p  &g_demo_packed=%p\n",
           (void*)&g_demo, (void*)&g_demo_packed);

    /* ---- Part C: union aliasing ---- */
    printf("\n== Part C: union aliasing ==\n");
    g_overlay.a[0] = 0x01; g_overlay.a[1] = 0x02; g_overlay.a[2] = 0x03;
    g_overlay.a[3] = 0x04; g_overlay.a[4] = 0x05;
    printf("sizeof(union overlay) = %u  (largest member wins)\n", (unsigned)sizeof(union overlay));
    printf("wrote a[] = 01 02 03 04 05\n");
    printf("read  b[] as shorts: b0=%04X b1=%04X b2=%04X  (little-endian)\n",
           (unsigned)g_overlay.b[0], (unsigned)g_overlay.b[1], (unsigned)g_overlay.b[2]);
    printf("inspect in Memory view: &g_overlay=%p\n", (void*)&g_overlay);

    /* ---- Part D: bitfield ---- */
    printf("\n== Part D: bitfield ==\n");
    g_ctrl.raw = 0;            /* clear all bits */
    g_ctrl.bits.enable    = 1; /* set bit 0 */
    g_ctrl.bits.clksource = 1; /* set bit 2 */
    printf("sizeof(union systick_ctrl) = %u\n", (unsigned)sizeof(union systick_ctrl));
    printf("set enable(bit0) + clksource(bit2) -> raw = 0x%08X  (expect 0x00000005)\n", g_ctrl.raw);
    printf("inspect in Memory view: &g_ctrl=%p\n", (void*)&g_ctrl);

    printf("\nDone. Use the printed addresses to explore the bytes in the Memory view.\n");

    // Bare-metal: never leave main.
    while (1);
    return 0;
}
