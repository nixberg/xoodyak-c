#ifndef keyed_xoodyak_h
#define keyed_xoodyak_h

#include "xoodyak.h"

typedef struct{
    Xoodyak xoodyak;
} KeyedXoodyak;

void keyed_xoodyak_init(KeyedXoodyak *keyed_xoodyak,
                        const void *key,
                        size_t key_len,
                        const void *id,
                        size_t id_len,
                        const void *counter,
                        size_t counter_len);

void keyed_xoodyak_absorb(KeyedXoodyak *keyed_xoodyak, const void *input, size_t input_len);

void keyed_xoodyak_encrypt(KeyedXoodyak *keyed_xoodyak,
                           const void *plaintext,
                           void *ciphertext,
                           size_t len);

void keyed_xoodyak_decrypt(KeyedXoodyak *keyed_xoodyak,
                           const void *ciphertext,
                           void *plaintext,
                           size_t len);

void keyed_xoodyak_squeeze(KeyedXoodyak *keyed_xoodyak, void *output, size_t output_len);

void keyed_xoodyak_squeeze_key(KeyedXoodyak *keyed_xoodyak, void *output, size_t output_len);

void keyed_xoodyak_ratchet(KeyedXoodyak *keyed_xoodyak);

#endif /* keyed_xoodyak_h */
