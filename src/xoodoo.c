#include <string.h>
#include <assert.h>

#include "xoodoo.h"

void xoodoo_init(Xoodoo *xoodoo) {
    memset(xoodoo->state, 0 , 48);
}

uint8_t xoodoo_get(const Xoodoo *xoodoo, size_t index) {
    assert(0 <= index && index < 48);
    return xoodoo->state[index];
}

void xoodoo_xor(Xoodoo *xoodoo, size_t index, uint8_t byte) {
    assert(0 <= index && index < 48);
    xoodoo->state[index] ^= byte;
}

static void unpack(const uint8_t source[48], uint32_t destination[12]) {
    for (size_t i = 0, j = 0; i < 12; i++) {
        destination[i]  = ((uint32_t)source[j++]) <<  0;
        destination[i] |= ((uint32_t)source[j++]) <<  8;
        destination[i] |= ((uint32_t)source[j++]) << 16;
        destination[i] |= ((uint32_t)source[j++]) << 24;
    }
}

static inline uint32_t rotate(uint32_t v, uint32_t n) {
    return (v >> n) | (v << (32 - n));
}

static inline void swap(uint32_t state[12], size_t i, size_t j) {
    uint32_t temp = state[i];
    state[i] = state[j];
    state[j] = temp;
}

static void round(uint32_t state[12], uint32_t round_constant) {
    uint32_t e[4];
    
    for (size_t i = 0; i < 4; i++) {
        e[i] = rotate(state[i] ^ state[i + 4] ^ state[i + 8], 18);
        e[i] ^= rotate(e[i], 9);
    }
    
    for (size_t i = 0; i < 12; i++) {
        state[i] ^= e[(i - 1) & 3];
    }
    swap(state, 7, 4);
    swap(state, 7, 5);
    swap(state, 7, 6);
    state[0] ^= round_constant;
    
    for (size_t i = 0; i < 4; i++) {
        const uint32_t a = state[i];
        const uint32_t b = state[i + 4];
        const uint32_t c = rotate(state[i + 8], 21);
        state[i + 8] = rotate((b & ~a) ^ c, 24);
        state[i + 4] = rotate((a & ~c) ^ b, 31);
        state[i] ^= c & ~b;
    }
    
    swap(state, 8, 10);
    swap(state, 9, 11);
}

static void pack(const uint32_t source[12], uint8_t destination[48]) {
    for (size_t i = 0, j = 0; i < 12; i++) {
        destination[j++] = (uint8_t)(source[i] >>  0);
        destination[j++] = (uint8_t)(source[i] >>  8);
        destination[j++] = (uint8_t)(source[i] >> 16);
        destination[j++] = (uint8_t)(source[i] >> 24);
    }
}

void xoodoo_permute(Xoodoo *xoodoo) {
    uint32_t state[12];
    
    unpack(xoodoo->state, state);
    
    round(state, 0x058);
    round(state, 0x038);
    round(state, 0x3c0);
    round(state, 0x0d0);
    round(state, 0x120);
    round(state, 0x014);
    round(state, 0x060);
    round(state, 0x02c);
    round(state, 0x380);
    round(state, 0x0f0);
    round(state, 0x1a0);
    round(state, 0x012);
    
    pack(state, xoodoo->state);
}
