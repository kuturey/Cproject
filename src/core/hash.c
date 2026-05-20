#include "../include/minigit.h"
#include <stdio.h>
#include <ctype.h>

void compute_hash(const void *data, int len, unsigned char *hash) {
    const unsigned char *bytes = (const unsigned char*)data;
    
    unsigned int state[5] = {
        0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0
    };
    
    //перемешивание
    unsigned char byte;
    unsigned int temp;
    for (int i = 0; i < len; i++) {
        byte = bytes[i];
        
        state[0] ^= (byte << 24) | (byte << 16) | (byte << 8) | byte;
        state[1] += byte * (i + 1);
        state[2] ^= state[1] + byte;
        state[3] = (state[3] << 7) | (state[3] >> 25);
        state[4] += state[0] ^ state[2];
        
        temp = state[0];
        state[0] = state[1];
        state[1] = state[2];
        state[2] = state[3];
        state[3] = state[4];
        state[4] = temp ^ state[2] ^ byte;
    }
    
    state[0] ^= len;
    state[1] += len << 8;
    state[2] ^= len << 16;
    state[3] += len << 24;
    state[4] ^= len;
    
    for (int r = 0; r < 3; r++) {
        for (int i = 0; i < 5; i++) {
            state[i] = (state[i] << 11) | (state[i] >> 21);
            state[i] ^= state[(i + 1) % 5];
        }
    }
    
    for (int i = 0; i < 5; i++) {
        hash[i*4] = (state[i] >> 24);
        hash[i*4 + 1] = (state[i] >> 16);
        hash[i*4 + 2] = (state[i] >> 8);
        hash[i*4 + 3] = state[i];
    }
    
    hash[0] ^= hash[7] ^ hash[15] ^ hash[19];
    hash[1] ^= hash[8] ^ hash[14] ^ hash[18];
}

int hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

int parse_hash(const char *s, unsigned char *hash) {
    if (!s || !hash || strlen(s) < SHA1_HASH_SIZE * 2) return 0;
    for (int i = 0; i < SHA1_HASH_SIZE; i++) {
        int hi = hex_value(s[i * 2]);
        int lo = hex_value(s[i * 2 + 1]);
        if (hi < 0 || lo < 0) return 0;
        hash[i] = (unsigned char)((hi << 4) | lo);
    }
    return 1;
}