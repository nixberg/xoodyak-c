#include <stdio.h>
#include <assert.h>
#include <strings.h>
#include <stdbool.h>

#include "../src/xoodoo.h"
#include "../src/xoodyak.h"

#include "hashkats.h"
#include "aeadkats.h"

static void test_xoodoo(void) {
    Xoodoo xoodoo;
    xoodoo_init(&xoodoo);
    
    for (size_t i = 0; i < 384; i++) {
        xoodoo_permute(&xoodoo);
    }
    
    uint8_t expected[48] = {
        0xb0, 0xfa, 0x04, 0xfe, 0xce, 0xd8, 0xd5, 0x42,
        0xe7, 0x2e, 0xc6, 0x29, 0xcf, 0xe5, 0x7a, 0x2a,
        0xa3, 0xeb, 0x36, 0xea, 0x0a, 0x9e, 0x64, 0x14,
        0x1b, 0x52, 0x12, 0xfe, 0x69, 0xff, 0x2e, 0xfe,
        0xa5, 0x6c, 0x82, 0xf1, 0xe0, 0x41, 0x4c, 0xfc,
        0x4f, 0x39, 0x97, 0x15, 0xaf, 0x2f, 0x09, 0xeb,
    };
    
    for (size_t i = 0; i < 48; i++) {
        assert(xoodoo.state[i] == expected[i]);
    }
    
    xoodoo_init(&xoodoo);
    
    for (size_t i = 0; i < 48; i++) {
        assert(xoodoo_get(&xoodoo, i) == 0);
        xoodoo_xor(&xoodoo, i, i);
        assert(xoodoo_get(&xoodoo, i) == i);
    }
}

static void hex_to_bytes(const char *hex, uint8_t *bytes, size_t bytes_len) {
    size_t hex_len = strlen(hex);
    assert(hex_len == 2 * bytes_len);
    
    const uint8_t map[] = {
      0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
      0x8, 0x9, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
      0x0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x0,
      0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    };
    
    for (size_t i = 0; i < bytes_len; i++) {
        uint8_t hi = ((uint8_t)hex[2 * i + 0] & 0x1F) ^ 0x10;
        uint8_t lo = ((uint8_t)hex[2 * i + 1] & 0x1F) ^ 0x10;
        bytes[i] = (map[hi] << 4) | map[lo];
    }
}

static void fill(uint8_t *bytes, size_t bytes_len) {
    for (size_t i = 0; i < bytes_len; i++) {
        bytes[i] = (uint8_t)(i & 0xff);
    }
}

/*static void print_bytes(uint8_t *bytes, size_t bytes_len) {
    for (size_t i = 0; i < bytes_len; i++) {
        printf("%02X", bytes[i]);
    }
    puts("");
}*/

static bool compare_bytes(uint8_t *a, uint8_t *b, size_t len) {
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum |= a[i] ^ b[i];
    }
    return (sum == 0);
}

static void test_xoodyak_hash(void) {
    Xoodyak xoodyak;
    uint8_t msg[HASH_KATS_LEN];
    uint8_t md[MD_LEN];
    uint8_t new_md[MD_LEN];
    
    for (size_t i = 0; i < HASH_KATS_LEN; i++) {
        fill(msg, i);
        hex_to_bytes(hash_kats[i], md, MD_LEN);
        
        xoodyak_init(&xoodyak);
        xoodyak_absorb(&xoodyak, msg, i);
        xoodyak_squeeze(&xoodyak, new_md, MD_LEN);
        
        assert(compare_bytes(md, new_md, MD_LEN));
    }
}

static void test_xoodyak_aead(void) {
    Xoodyak xoodyak;
    
    uint8_t key[KEY_LEN];
    uint8_t nonce[NONCE_LEN];
    uint8_t pt[PT_MAX_LEN];
    uint8_t new_pt[PT_MAX_LEN];
    uint8_t ad[AD_MAX_LEN];
    uint8_t ct[CT_MAX_LEN];
    uint8_t new_ct[CT_MAX_LEN];
    uint8_t new_tag[TAG_LEN];
    
    fill(key, KEY_LEN);
    fill(nonce, NONCE_LEN);
    
    assert((PT_MAX_LEN + 1) * (AD_MAX_LEN + 1) == AEAD_KATS_LEN);
    
    for (size_t pt_len = 0; pt_len <= PT_MAX_LEN; pt_len++) {
        fill(pt, pt_len);
        
        for (size_t ad_len = 0; ad_len <= AD_MAX_LEN; ad_len++) {
            fill(ad, ad_len);
            
            size_t ct_len = pt_len + TAG_LEN;
            hex_to_bytes(aead_kats[pt_len * (AD_MAX_LEN + 1) + ad_len], ct, ct_len);
            
            xoodyak_keyed(&xoodyak, key, KEY_LEN, NULL, 0, NULL, 0);
            xoodyak_absorb(&xoodyak, nonce, NONCE_LEN);
            xoodyak_absorb(&xoodyak, ad, ad_len);
            xoodyak_encrypt(&xoodyak, pt, new_ct, pt_len);
            xoodyak_squeeze(&xoodyak, new_ct + pt_len, TAG_LEN);
            
            assert(compare_bytes(ct, new_ct, ct_len));
            
            xoodyak_keyed(&xoodyak, key, KEY_LEN, NULL, 0, NULL, 0);
            xoodyak_absorb(&xoodyak, nonce, NONCE_LEN);
            xoodyak_absorb(&xoodyak, ad, ad_len);
            xoodyak_decrypt(&xoodyak, ct, new_pt, pt_len);
            xoodyak_squeeze(&xoodyak, new_tag, TAG_LEN);
            
            assert(compare_bytes(pt, new_pt, pt_len));
            assert(compare_bytes(ct + pt_len, new_tag, TAG_LEN));
        }
    }
}

int main() {
    test_xoodoo();
    test_xoodyak_hash();
    test_xoodyak_aead();
    return 0;
}
