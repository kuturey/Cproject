#include "../include/minigit.h"
#include <stdio.h>

// Улучшенная хеш-функция с равномерным распределением
void compute_hash(const void *data, size_t len, unsigned char *hash) {
    const unsigned char *bytes = (const unsigned char*)data;
    
    // Используем 5 разных состояний
    unsigned int state[5] = {
        0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0
    };
    
    // Обрабатываем каждый байт
    for (size_t i = 0; i < len; i++) {
        unsigned char byte = bytes[i];
        
        // Сложные операции для хорошего перемешивания
        state[0] ^= (byte << 24) | (byte << 16) | (byte << 8) | byte;
        state[1] += byte * (i + 1);
        state[2] ^= state[1] + byte;
        state[3] = (state[3] << 7) | (state[3] >> 25);
        state[4] += state[0] ^ state[2];
        
        // Меняем местами для дополнительного перемешивания
        unsigned int temp = state[0];
        state[0] = state[1];
        state[1] = state[2];
        state[2] = state[3];
        state[3] = state[4];
        state[4] = temp ^ state[2] ^ byte;
    }
    
    // Добавляем длину для уникальности
    state[0] ^= len;
    state[1] += len << 8;
    state[2] ^= len << 16;
    state[3] += len << 24;
    state[4] ^= len;
    
    // Финальное перемешивание
    for (int r = 0; r < 3; r++) {
        for (int i = 0; i < 5; i++) {
            state[i] = (state[i] << 11) | (state[i] >> 21);
            state[i] ^= state[(i + 1) % 5];
        }
    }
    
    // Записываем в хеш (20 байт)
    for (int i = 0; i < 5; i++) {
        hash[i*4]     = (state[i] >> 24) & 0xFF;
        hash[i*4 + 1] = (state[i] >> 16) & 0xFF;
        hash[i*4 + 2] = (state[i] >> 8) & 0xFF;
        hash[i*4 + 3] = state[i] & 0xFF;
    }
    
    // Гарантируем, что первый байт распределен равномерно
    hash[0] ^= hash[7] ^ hash[15] ^ hash[19];
    hash[1] ^= hash[8] ^ hash[14] ^ hash[18];
}