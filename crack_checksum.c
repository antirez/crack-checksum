/* Copyright (C) 2023 Salvatore Sanfilippo <antirez@gmail.com>
 * All Rights Reserved.
 *
 * This software is released under the BSD 2 clause license.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* CRC8 with the specified initialization value 'init' and
 * polynomial 'poly'. */
uint8_t crc8(const uint8_t *data, size_t len, uint8_t init, uint8_t poly)
{
    uint8_t crc = init;
    size_t i, j;
    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if ((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ poly);
            else
                crc <<= 1;
        }
    }
    return crc;
}

/* Bytes xor: init ^ byte[0] ^ byte[1] ... */
uint8_t xor8(const uint8_t *data, size_t len, uint8_t init) {
    uint8_t res = init;
    for (size_t j = 0; j < len; j++) res ^= data[j];
    return res;
}

/* Bytes sum mod 256: (init + byte[0] + byte[1] + byte[N]) MOD 256*/
uint8_t sum8(const uint8_t *data, size_t len, uint8_t init) {
    uint8_t res = init;
    for (size_t j = 0; j < len; j++) res += data[j];
    return res;
}

int bruteforce_crc8(const uint8_t **v, size_t numv, size_t len, size_t minlen) {
    for (size_t end = len-1; end > 0; end--) {
        for (size_t start = 0; start < len; start++) {
            uint8_t l = end-start+1;
            if (start >= end || l < minlen) continue;
            for (uint16_t poly = 0; poly < 256; poly++) {
                if ((poly & 1) == 0) continue; /* Must be odd. */
                for (uint16_t init = 0; init < 256; init++) {
                    size_t j;

                    /* Try CRC8 */
                    for (j = 0; j < numv; j++) {
                        if (crc8(v[j]+start,l-1,init,poly) !=
                            v[j][end]) break; /* No match. */
                    }
                    if (j == numv) {
                        printf("CRC8 MATCH start=%d end=%d init=0x%02X "
                               "poly=0x%02X\n",
                               (int)start, (int)end, (int)init, (int)poly);
                        return 1;
                    }

                    /* Try XOR */
                    for (j = 0; j < numv; j++) {
                        if (xor8(v[j]+start,l-1,init) !=
                            v[j][end]) break; /* No match. */
                    }
                    if (j == numv) {
                        printf("XOR8 MATCH start=%d end=%d init=0x%02X\n",
                               (int)start, (int)end, (int)init);
                        return 1;
                    }

                    /* Try adding modulo 256 */
                    for (j = 0; j < numv; j++) {
                        if (sum8(v[j]+start,l-1,init) !=
                            v[j][end]) break; /* No match. */
                    }
                    if (j == numv) {
                        printf("SUM8 MATCH start=%d end=%d init=0x%02X\n",
                               (int)start, (int)end, (int)init);
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int main(void) {
    /* Toyota TMPS protocol. */
    const uint8_t *vectors[] = {
        (uint8_t*)"\xD5\x15\x7B\x93\xCE\x9C\x00\x62\x3D",
        (uint8_t*)"\xD9\xB6\x22\x85\xD5\x9C\x80\x54\x8A",
        (uint8_t*)"\xD3\xC7\x1F\xD3\xDF\x1E\x00\x41\x96"
    };
    bruteforce_crc8(vectors,3,9,5);
    return 0;
}
