#ifndef __GPIO_H__
#define __GPIO_H__

#include "types.hpp"

#define PERIPH_BASE_RPI2                         0x3f000000

typedef struct
{
    uint32_t fsel[6];                            // GPIO Function Select
    uint32_t resvd_0x18;
    uint32_t set[2];                             // GPIO Pin Output Set
    uint32_t resvd_0x24;
    uint32_t clr[2];                             // GPIO Pin Output Clear
    uint32_t resvd_0x30;
    uint32_t lev[2];                             // GPIO Pin Level
    uint32_t resvd_0x3c;
    uint32_t eds[2];                             // GPIO Pin Event Detect Status
    uint32_t resvd_0x48;
    uint32_t ren[2];                             // GPIO Pin Rising Edge Detect Enable
    uint32_t resvd_0x54;
    uint32_t fen[2];                             // GPIO Pin Falling Edge Detect Enable
    uint32_t resvd_0x60;
    uint32_t hen[2];                             // GPIO Pin High Detect Enable
    uint32_t resvd_0x6c;
    uint32_t len[2];                             // GPIO Pin Low Detect Enable
    uint32_t resvd_0x78;
    uint32_t aren[2];                            // GPIO Pin Async Rising Edge Detect
    uint32_t resvd_0x84;
    uint32_t afen[2];                            // GPIO Pin Async Falling Edge Detect
    uint32_t resvd_0x90;
    uint32_t pud;                                // GPIO Pin Pull up/down Enable
    uint32_t pudclk[2];                          // GPIO Pin Pull up/down Enable Clock
    uint32_t resvd_0xa0[4];
    uint32_t test;
} __attribute__((packed, aligned(4))) gpio_t;


#define GPIO_OFFSET                              (0x00200000)


static inline void gpio_function_set(volatile gpio_t *gpio, uint8_t pin, uint8_t function)
{
    int regnum = pin / 10;
    int offset = (pin % 10) * 3;
    uint8_t funcmap[] = { 4, 5, 6, 7, 3, 2, 1, 0 };  // See datasheet for mapping

    if (function > 7)
    {
        return;
    }

    gpio->fsel[regnum] &= ~(0x7 << offset);
    gpio->fsel[regnum] |= ((funcmap[function]) << offset);
}

static inline void gpio_level_set(volatile gpio_t *gpio, uint8_t pin, uint8_t level)
{
    int regnum = pin >> 5;
    int offset = (pin & 0x1f);

    if (level)
    {
        gpio->set[regnum] = (1 << offset);
    }
    else
    {
        gpio->clr[regnum] = (1 << offset);
    }
}

static inline void gpio_level_get(volatile gpio_t *gpio, uint8_t pin, uint8_t* level)
{
    int regnum = pin >> 5;
    int offset = (pin & 0x1f);
    int pinValue = (gpio->lev[regnum] >> offset) & 0x01;

    *level = pinValue;
}

static inline void gpio_output_set(volatile gpio_t *gpio, uint8_t pin, uint8_t output)
{
    int regnum = pin / 10;
    int offset = (pin % 10) * 3;
    uint8_t function = output ? 1 : 0;  // See datasheet for mapping

    gpio->fsel[regnum] &= ~(0x7 << offset);
    gpio->fsel[regnum] |= ((function & 0x7) << offset);
}

#endif

