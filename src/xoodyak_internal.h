#ifndef xoodyak_internal_h
#define xoodyak_internal_h

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "xoodyak.h"

void down(Xoodyak *xoodyak, const uint8_t *block, size_t block_len, XoodyakFlag flag);

void up(Xoodyak *xoodyak, uint8_t *block, size_t block_len, XoodyakFlag);

void absorb_any(Xoodyak *xoodyak,
                const uint8_t *input,
                size_t input_len,
                size_t rate,
                XoodyakFlag down_flag);

void squeeze_any(Xoodyak *xoodyak, uint8_t *output, size_t output_len, XoodyakFlag up_flag);

static inline void precondition(bool condition, const char *message) {
    if (!condition) {
        puts(message);
        exit(EXIT_FAILURE);
    }
}

static inline size_t min(size_t a, size_t b) {
    return (a < b) ? a : b;
}

#endif /* xoodyak_internal_h */
