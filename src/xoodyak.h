#ifndef xoodyak_h
#define xoodyak_h

#include <stdint.h>

typedef enum {
    Zero       = 0x00,
    AbsorbKey  = 0x02,
    Absorb     = 0x03,
    Ratchet    = 0x10,
    SqueezeKey = 0x20,
    Squeeze    = 0x40,
    Crypt      = 0x80,
} XoodyakFlag;

typedef enum {
    Hash,
    Keyed,
} XoodyakMode;

typedef struct {
    size_t absorb;
    size_t squeeze;
} XoodyakRates;

typedef enum {
    Up,
    Down,
} XoodyakPhase;

typedef struct{
    XoodyakMode mode;
    XoodyakRates rates;
    XoodyakPhase phase;
    Xoodoo xoodoo;
} Xoodyak;

void xoodyak_init(Xoodyak *xoodyak);

void xoodyak_keyed(Xoodyak *xoodyak, const void *key, size_t key_len, const void *id, size_t id_len, const void *counter, size_t counter_len);

void xoodyak_absorb(Xoodyak *xoodyak, const void *input, size_t input_len);

void xoodyak_encrypt(Xoodyak *xoodyak, const void *plaintext, void *ciphertext, size_t len);

void xoodyak_decrypt(Xoodyak *xoodyak, const void *ciphertext, void *plaintext, size_t len);

void xoodyak_squeeze(Xoodyak *xoodyak, void *output, size_t output_len);

void xoodyak_squeeze_key(Xoodyak *xoodyak, void *output, size_t output_len);

void xoodyak_ratchet(Xoodyak *xoodyak);

#endif /* xoodyak_h */
