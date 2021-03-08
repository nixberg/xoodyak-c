#include "xoodoo_internal.h"
#include "xoodyak_internal.h"

void down(Xoodyak *xoodyak, const uint8_t *block, size_t block_len, XoodyakFlag flag) {
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

void up(Xoodyak *xoodyak, uint8_t *block, size_t block_len, XoodyakFlag flag) {
    xoodyak->phase = Up;
    if (xoodyak->mode != Hash) {
        xoodoo_xor(&xoodyak->xoodoo, 47, flag);
    }
    xoodoo_permute(&xoodyak->xoodoo);
    for (size_t i = 0; i < block_len; i++) {
        block[i] = xoodoo_get(&xoodyak->xoodoo, i);
    }
}

void absorb_any(Xoodyak *xoodyak,
                const uint8_t *input,
                size_t input_len,
                size_t rate,
                XoodyakFlag down_flag)
{
    do {
        size_t block_size = min(rate, input_len);
        
        if (xoodyak->phase != Up) {
            up(xoodyak, NULL, 0, Zero);
        }
        down(xoodyak, input, block_size, down_flag);
        down_flag = Zero;
        
        input     += block_size;
        input_len -= block_size;
        
    } while (input_len > 0);
}

void squeeze_any(Xoodyak *xoodyak, uint8_t *output, size_t output_len, XoodyakFlag up_flag) {
    size_t block_size = min(xoodyak->rates.squeeze, output_len);
    
    up(xoodyak, output, block_size, up_flag);
    
    output     += block_size;
    output_len -= block_size;
    
    while (output_len > 0) {
        block_size = min(xoodyak->rates.squeeze, output_len);
        
        down(xoodyak, NULL, 0, Zero);
        up(xoodyak, output, block_size, Zero);
        
        output     += block_size;
        output_len -= block_size;
    }
}
