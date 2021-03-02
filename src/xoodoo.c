#include <assert.h>
#include <string.h>

#include "xoodoo.h"

static inline void unpack(const uint8_t source[48], uint32_t destination[12]);
static inline void pack(const uint32_t source[12], uint8_t destination[48]);
static inline uint32_t rotate_left(uint32_t x, uint32_t n);
static inline void swap(uint32_t state[12], size_t i, size_t j);

void xoodoo_init(Xoodoo *xoodoo) {
    memset(xoodoo->bytes, 0, 48);
}

uint8_t xoodoo_get(const Xoodoo *xoodoo, size_t index) {
    assert(index < 48);
    return xoodoo->bytes[index];
}

void xoodoo_xor(Xoodoo *xoodoo, size_t index, uint8_t byte) {
    assert(index < 48);
    xoodoo->bytes[index] ^= byte;
}

void xoodoo_permute(Xoodoo *xoodoo) {
    uint32_t state[12];
    
    unpack(xoodoo->bytes, state);
    
    const uint32_t round_constants[12] = {
        0x058, 0x038, 0x3c0, 0x0d0,
        0x120, 0x014, 0x060, 0x02c,
        0x380, 0x0f0, 0x1a0, 0x012,
    };
    
    for (size_t round = 0; round < 12; round++) {
        uint32_t e[4];
        
        for (size_t i = 0; i < 4; i++) {
            uint32_t p = state[i] ^ state[i + 4] ^ state[i + 8];
            e[i] = rotate_left(p, 5) ^ rotate_left(p, 14);
        }
        
        for (size_t i = 0; i < 12; i++) {
            state[i] ^= e[(i - 1) & 3];
        }
        
        swap(state, 7, 4);
        swap(state, 7, 5);
        swap(state, 7, 6);
        
        state[0] ^= round_constants[round];
        
        for (size_t i = 0; i < 4; i++) {
            const uint32_t a = state[i + 0];
            const uint32_t b = state[i + 4];
            const uint32_t c = rotate_left(state[i + 8], 11);
            
            state[i + 8] = rotate_left((b & ~a) ^ c, 8);
            state[i + 4] = rotate_left((a & ~c) ^ b, 1);
            state[i + 0] ^= c & ~b;
        }
        
        swap(state, 8, 10);
        swap(state, 9, 11);
    }
    
    pack(state, xoodoo->bytes);
}

static inline void unpack(const uint8_t source[48], uint32_t destination[12]) {
    for (size_t i = 0, j = 0; i < 12; i++) {
        destination[i]  = ((uint32_t)source[j++]) <<  0;
        destination[i] |= ((uint32_t)source[j++]) <<  8;
        destination[i] |= ((uint32_t)source[j++]) << 16;
        destination[i] |= ((uint32_t)source[j++]) << 24;
    }
}

static inline void pack(const uint32_t source[12], uint8_t destination[48]) {
    for (size_t i = 0, j = 0; i < 12; i++) {
        destination[j++] = (uint8_t)(source[i] >>  0);
        destination[j++] = (uint8_t)(source[i] >>  8);
        destination[j++] = (uint8_t)(source[i] >> 16);
        destination[j++] = (uint8_t)(source[i] >> 24);
    }
}

static inline uint32_t rotate_left(uint32_t x, uint32_t n) {
    return (x << n) | (x >> (32 - n));
}

static inline void swap(uint32_t state[12], size_t i, size_t j) {
    uint32_t temp = state[i];
    state[i] = state[j];
    state[j] = temp;
}
