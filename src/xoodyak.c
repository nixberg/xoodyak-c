#include "xoodyak.h"

#include "xoodoo_internal.h"
#include "xoodyak_internal.h"

static const size_t RATE_HASH = 16;

void xoodyak_init(Xoodyak *xoodyak) {
    xoodoo_init(&xoodyak->xoodoo);
    xoodyak->rates.absorb = RATE_HASH;
    xoodyak->rates.squeeze = RATE_HASH;
    xoodyak->phase = Up;
    xoodyak->mode = Hash;
}

void xoodyak_absorb(Xoodyak *xoodyak, const void *input, size_t input_len) {
    absorb_any(xoodyak, input, input_len, xoodyak->rates.absorb, Absorb);
}

void xoodyak_squeeze(Xoodyak *xoodyak, void *output, size_t output_len) {
    squeeze_any(xoodyak, output, output_len, Squeeze);
}
