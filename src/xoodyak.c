#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "xoodoo.h"
#include "xoodyak.h"

const static size_t RATES_HASH    = 16;
const static size_t RATES_INPUT   = 44;
const static size_t RATES_OUTPUT  = 24;
const static size_t RATES_RATCHET = 16;

static void ensure(bool condition, const char *message) {
    if (!condition) {
        puts(message);
        exit(EXIT_FAILURE);
    }
}

static void down(Xoodyak *xoodyak, const uint8_t *block, size_t block_len, XoodyakFlag flag) {
    xoodyak->phase = Down;
    for (size_t i = 0; i < block_len; i++) {
        xoodoo_xor(&xoodyak->xoodoo, i, block[i]);
    }
    xoodoo_xor(&xoodyak->xoodoo, block_len, 0x01);
    if (xoodyak->mode == Hash) {
        xoodoo_xor(&xoodyak->xoodoo, 47, flag & 0x01);
    } else {
        xoodoo_xor(&xoodyak->xoodoo, 47, flag);
    }
}

// TODO: simplify in other impls?
static void up(Xoodyak *xoodyak, uint8_t *block, size_t block_len, XoodyakFlag flag) {
    xoodyak->phase = Up;
    if (xoodyak->mode != Hash) {
        xoodoo_xor(&xoodyak->xoodoo, 47, flag);
    }
    xoodoo_permute(&xoodyak->xoodoo);
    for (size_t i = 0; i < block_len; i++) {
        block[i] = xoodoo_get(&xoodyak->xoodoo, i);
    }
}

static inline size_t min(size_t a, size_t b) {
    return (a < b) ? a : b;
}

static void xoodyak_absorb_any(Xoodyak *xoodyak, const uint8_t *input, size_t input_len, size_t rate, XoodyakFlag down_flag) {
    do {
        size_t block_size = min(input_len, rate);
        
        if (xoodyak->phase != Up) {
            up(xoodyak, NULL, 0, Zero);
        }
        down(xoodyak, input, block_size, down_flag);
        down_flag = Zero;
        
        input     += block_size;
        input_len -= block_size;
        
    } while (input_len > 0);
}

static void xoodyak_absorb_key(Xoodyak *xoodyak, const void *key, size_t key_len, const void *id, size_t id_len, const void *counter, size_t counter_len) {
    ensure(key_len + id_len <= RATES_INPUT - 1, "xoodyak_keyed: Key + ID too long!");

    xoodyak->mode = Keyed;
    xoodyak->rates.absorb = RATES_INPUT;
    xoodyak->rates.squeeze = RATES_OUTPUT;
    
    size_t buffer_len = 0;
    uint8_t buffer[RATES_INPUT];
    memcpy(buffer, key, key_len);
    buffer_len += key_len;
    memcpy(buffer + buffer_len, id, id_len);
    buffer_len += id_len;
    buffer[buffer_len] = (uint8_t)id_len;
    buffer_len += 1;

    xoodyak_absorb_any(xoodyak, buffer, buffer_len, xoodyak->rates.absorb, AbsorbKey);
    
    if (counter_len > 0) {
        xoodyak_absorb_any(xoodyak, counter, counter_len, 1, Zero);
    }
}


static void xoodyak_crypt(Xoodyak *xoodyak, const uint8_t *input, uint8_t *output, size_t len, bool decrypt) {
    XoodyakFlag flag = Crypt;
    
    do {
        size_t block_size = min(len, RATES_OUTPUT);
           
        up(xoodyak, NULL, 0, flag);
        flag = Zero;
        
        for (size_t i = 0; i < block_size; i++) {
            output[i] = input[i] ^ xoodoo_get(&xoodyak->xoodoo, i);
        }
        
        if (decrypt) {
            down(xoodyak, output, block_size, Zero);
        } else {
            down(xoodyak, input, block_size, Zero);
        }
        
        input  += block_size;
        output += block_size;
        len    -= block_size;
        
    } while (len > 0);
}

static void xoodyak_squeeze_any(Xoodyak *xoodyak, uint8_t *output, size_t output_len, XoodyakFlag up_flag) {
    size_t block_size = min(output_len, xoodyak->rates.squeeze);
    
    up(xoodyak, output, block_size, up_flag);
    
    output     += block_size;
    output_len -= block_size;
    
    while (output_len > 0) {
        block_size = min(output_len, xoodyak->rates.squeeze);
        
        down(xoodyak, NULL, 0, Zero);
        up(xoodyak, output, block_size, Zero);
        
        output     += block_size;
        output_len -= block_size;
    }
}

void xoodyak_init(Xoodyak *xoodyak) {
    xoodyak->mode = Hash;
    xoodyak->rates.absorb = RATES_HASH;
    xoodyak->rates.squeeze = RATES_HASH;
    xoodyak->phase = Up;
    xoodoo_init(&xoodyak->xoodoo);
}

void xoodyak_keyed(Xoodyak *xoodyak, const void *key, size_t key_len, const void *id, size_t id_len, const void *counter, size_t counter_len) {
    xoodyak_init(xoodyak);
    xoodyak_absorb_key(xoodyak, key, key_len, id, id_len, counter, counter_len);
}

void xoodyak_absorb(Xoodyak *xoodyak, const void *input, size_t input_len) {
    xoodyak_absorb_any(xoodyak, input, input_len, xoodyak->rates.absorb, Absorb);
}

void xoodyak_encrypt(Xoodyak *xoodyak, const void *plaintext, void *ciphertext, size_t len) {
    ensure(xoodyak->mode == Keyed, "xoodyak_encrypt: Not in keyed mode!");
    xoodyak_crypt(xoodyak, plaintext, ciphertext, len, false);
}

void xoodyak_decrypt(Xoodyak *xoodyak, const void *ciphertext, void *plaintext, size_t len) {
    ensure(xoodyak->mode == Keyed, "xoodyak_decrypt: Not in keyed mode!");
    xoodyak_crypt(xoodyak, ciphertext, plaintext, len, true);
}

void xoodyak_squeeze(Xoodyak *xoodyak, void *output, size_t output_len) {
    xoodyak_squeeze_any(xoodyak, output, output_len, Squeeze);
}

void xoodyak_squeeze_key(Xoodyak *xoodyak, void *output, size_t output_len) {
    ensure(xoodyak->mode == Keyed, "xoodyak_squeeze_key: Not in keyed mode!");
    xoodyak_squeeze_any(xoodyak, output, output_len, SqueezeKey);
}

void xoodyak_ratchet(Xoodyak *xoodyak) {
    ensure(xoodyak->mode == Keyed, "xoodyak_ratchet: Not in keyed mode!");
    uint8_t buffer[RATES_RATCHET]; // TODO: Analyzer complains?
    xoodyak_squeeze_any(xoodyak, buffer, RATES_RATCHET, Ratchet);
    xoodyak_absorb_any(xoodyak, buffer, RATES_RATCHET, xoodyak->rates.absorb, Zero);
}
