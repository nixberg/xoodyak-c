#include <string.h>
#include <assert.h>

#include "xoodoo.h"

void xoodoo_init(Xoodoo *xoodoo) {
    memset(xoodoo->state, 0 , 48);
}

uint8_t xoodoo_get(const Xoodoo *xoodoo, size_t index) {
    assert(0 <= index && index < 48);
    const uint8_t *bytes = (const void *)(xoodoo->state);
    return bytes[index];
}

void xoodoo_xor(Xoodoo *xoodoo, size_t index, uint8_t byte) {
    assert(0 <= index && index < 48);
    uint8_t *bytes = (void *)(xoodoo->state);
    bytes[index] ^= byte;
}

static inline void swap_bytes_if_big_endian(Xoodoo *xoodoo) {
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    for (size_t i = 0; i < 12; i++) {
        xoodoo->state[i] = __builtin_bswap32(xoodoo->state[i]);
    }
#else
    (void)xoodoo;
#endif
}

static inline uint32_t rotate(uint32_t v, uint32_t n) {
    return (v >> n) | (v << (32 - n));
}

static inline void swap(Xoodoo *xoodoo, size_t i, size_t j) {
    uint32_t temp = xoodoo->state[i];
    xoodoo->state[i] = xoodoo->state[j];
    xoodoo->state[j] = temp;
}

void xoodoo_permute(Xoodoo *xoodoo) {
    swap_bytes_if_big_endian(xoodoo);

    const uint32_t round_constants[12] = {
        0x058, 0x038, 0x3c0, 0x0d0,
        0x120, 0x014, 0x060, 0x02c,
        0x380, 0x0f0, 0x1a0, 0x012,
    };
    
    for (size_t round = 0; round < 12; round++) {
        uint32_t e[4];
        
        for (size_t i = 0; i < 4; i++) {
            e[i] = rotate(xoodoo->state[i] ^ xoodoo->state[i + 4] ^ xoodoo->state[i + 8], 18);
            e[i] ^= rotate(e[i], 9);
        }
        
        for (size_t i = 0; i < 12; i++) {
            xoodoo->state[i] ^= e[(i - 1) & 3];
        }

        swap(xoodoo, 7, 4);
        swap(xoodoo, 7, 5);
        swap(xoodoo, 7, 6);
        xoodoo->state[0] ^= round_constants[round];
        
        for (size_t i = 0; i < 4; i++) {
            const uint32_t a = xoodoo->state[i];
            const uint32_t b = xoodoo->state[i + 4];
            const uint32_t c = rotate(xoodoo->state[i + 8], 21);

            xoodoo->state[i + 8] = rotate((b & ~a) ^ c, 24);
            xoodoo->state[i + 4] = rotate((a & ~c) ^ b, 31);
            xoodoo->state[i] ^= c & ~b;
        }
        
        swap(xoodoo, 8, 10);
        swap(xoodoo, 9, 11);
    }
    
    swap_bytes_if_big_endian(xoodoo);
}
