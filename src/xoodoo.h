#ifndef xoodoo_h
#define xoodoo_h

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t bytes[48];
} Xoodoo;

void xoodoo_init(Xoodoo *xoodoo);

uint8_t xoodoo_get(const Xoodoo *xoodoo, size_t index);

void xoodoo_xor(Xoodoo *xoodoo, size_t index, uint8_t byte);

void xoodoo_permute(Xoodoo *xoodoo);

#endif /* xoodoo_h */
