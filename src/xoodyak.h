#ifndef xoodyak_h
#define xoodyak_h

#include "xoodoo.h"

typedef enum {
    Up,
    Down,
} XoodyakPhase;

typedef enum {
    Hash,
    Keyed,
} XoodyakMode;

typedef struct {
    size_t absorb;
    size_t squeeze;
} XoodyakRates;

typedef enum {
    Zero       = 0x00,
    AbsorbKey  = 0x02,
    Absorb     = 0x03,
    Ratchet    = 0x10,
    SqueezeKey = 0x20,
    Squeeze    = 0x40,
    Crypt      = 0x80,
} XoodyakFlag;

typedef struct{
    Xoodoo xoodoo;
    XoodyakRates rates;
    XoodyakPhase phase;
    XoodyakMode mode;
} Xoodyak;

void xoodyak_init(Xoodyak *xoodyak);

void xoodyak_absorb(Xoodyak *xoodyak, const void *input, size_t input_len);

void xoodyak_squeeze(Xoodyak *xoodyak, void *output, size_t output_len);

#endif /* xoodyak_h */
