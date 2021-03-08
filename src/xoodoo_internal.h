#ifndef xoodoo_internal_h
#define xoodoo_internal_h

#include "xoodyak.h"

void xoodoo_init(Xoodoo *xoodoo);

uint8_t xoodoo_get(const Xoodoo *xoodoo, size_t index);

void xoodoo_xor(Xoodoo *xoodoo, size_t index, uint8_t byte);

void xoodoo_permute(Xoodoo *xoodoo);

#endif /* xoodoo_internal_h */
