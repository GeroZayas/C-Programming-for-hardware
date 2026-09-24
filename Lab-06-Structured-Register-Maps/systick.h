#pragma once

#include "stdint.h"

typedef volatile struct __attribute__((packed)) {
    struct {
        uint32_t enable : 1;
        uint32_t tick_int : 1;
        uint32_t clksource : 1;
        uint32_t reserved : 13;
        uint32_t count_flag : 1;
    } ctrl;
    uint32_t load;
    uint32_t val;
    uint32_t calib;
} systick_t;

#define SYSTICK_BASE 0xE000E010
#define SYSTICK ((systick_t *)SYSTICK_BASE)