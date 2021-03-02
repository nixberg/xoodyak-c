#include <string.h>

#include "xoodoo.h"
#include "xoodyak.h"
#include "keyed_xoodyak.h"
#include "xoodyak_internal.h"

static const size_t RATE_KEYED_INPUT  = 44;
static const size_t RATE_KEYED_OUTPUT = 24;
static const size_t RATE_RATCHET      = 16;
static const size_t RATE_COUNTER      = 1;

static void crypt(Xoodyak *xoodyak,
                  const uint8_t *input,
                  uint8_t *output,
                  size_t len,
                  bool decrypt);

void keyed_xoodyak_init(KeyedXoodyak *keyed_xoodyak,
                        const void *key,
                        size_t key_len,
                        const void *id,
                        size_t id_len,
                        const void *counter,
                        size_t counter_len)
{
    precondition(key != NULL && key_len > 0, "keyed_xoodyak_init: empty key");
    precondition(key_len + id_len + 1 <= RATE_KEYED_INPUT,
                 "keyed_xoodyak_init: key_len + id_len must no exceed 43 bytes");
    
    Xoodyak *xoodyak = &keyed_xoodyak->xoodyak;
    
    xoodoo_init(&xoodyak->xoodoo);
    xoodyak->rates.absorb = RATE_KEYED_INPUT;
    xoodyak->rates.squeeze = RATE_KEYED_OUTPUT;
    xoodyak->phase = Up;
    xoodyak->mode = Keyed;
    
    size_t buffer_len = 0;
    uint8_t buffer[RATE_KEYED_INPUT];
    memcpy(buffer, key, key_len);
    buffer_len += key_len;
    memcpy(buffer + buffer_len, id, id_len);
    buffer_len += id_len;
    buffer[buffer_len] = (uint8_t)id_len;
    buffer_len += 1;
    
    absorb_any(xoodyak, buffer, buffer_len, xoodyak->rates.absorb, AbsorbKey);
    
    if (counter_len > 0) {
        absorb_any(xoodyak, counter, counter_len, RATE_COUNTER, Zero);
    }
}

void keyed_xoodyak_absorb(KeyedXoodyak *keyed_xoodyak, const void *input, size_t input_len) {
    xoodyak_absorb(&keyed_xoodyak->xoodyak, input, input_len);
}

void keyed_xoodyak_encrypt(KeyedXoodyak *keyed_xoodyak,
                           const void *plaintext,
                           void *ciphertext,
                           size_t len)
{
    crypt(&keyed_xoodyak->xoodyak, plaintext, ciphertext, len, false);
}

void keyed_xoodyak_decrypt(KeyedXoodyak *keyed_xoodyak,
                           const void *ciphertext,
                           void *plaintext,
                           size_t len)
{
    crypt(&keyed_xoodyak->xoodyak, ciphertext, plaintext, len, true);
}

void keyed_xoodyak_squeeze(KeyedXoodyak *keyed_xoodyak, void *output, size_t output_len) {
    xoodyak_squeeze(&keyed_xoodyak->xoodyak, output, output_len);
}

void keyed_xoodyak_squeeze_key(KeyedXoodyak *keyed_xoodyak, void *output, size_t output_len) {
    squeeze_any(&keyed_xoodyak->xoodyak, output, output_len, SqueezeKey);
}

void keyed_xoodyak_ratchet(KeyedXoodyak *keyed_xoodyak) {
    uint8_t buffer[RATE_RATCHET];
    Xoodyak *xoodyak = &keyed_xoodyak->xoodyak;
    squeeze_any(xoodyak, buffer, RATE_RATCHET, Ratchet);
    absorb_any(xoodyak, buffer, RATE_RATCHET, xoodyak->rates.absorb, Zero);
}

static void crypt(Xoodyak *xoodyak,
                  const uint8_t *input,
                  uint8_t *output,
                  size_t len,
                  bool decrypt)
{
    XoodyakFlag flag = Crypt;
    
    do {
        size_t block_size = min(len, RATE_KEYED_OUTPUT);
           
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
