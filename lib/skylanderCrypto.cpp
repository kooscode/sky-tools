/*
 * Skylander Encryption and Checksum Library - Implementation
 *
 * AES-128 and MD5 implementations based on public domain code.
 * CRC-16 and Skylander-specific logic based on community reverse engineering.
 */

#include "skylanderCrypto.hpp"
#include <cstring>

namespace xk
{
    // ==================== CRC-16/CCITT-FALSE ====================

    uint16_t skylanderCrypto::crc16(const uint8_t* data, size_t length, uint16_t init)
    {
        uint16_t crc = init;
        const uint16_t poly = 0x1021;

        for (size_t i = 0; i < length; i++)
        {
            crc ^= (static_cast<uint16_t>(data[i]) << 8);
            for (int j = 0; j < 8; j++)
            {
                if (crc & 0x8000)
                    crc = (crc << 1) ^ poly;
                else
                    crc <<= 1;
            }
        }
        return crc;
    }

    // ==================== MD5 Implementation ====================

    // MD5 constants
    static const uint32_t MD5_K[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    };

    static const uint32_t MD5_S[64] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
    };

    static inline uint32_t leftRotate(uint32_t x, uint32_t c)
    {
        return (x << c) | (x >> (32 - c));
    }

    void skylanderCrypto::md5(const uint8_t* data, size_t length, uint8_t* hash)
    {
        // Initialize hash values
        uint32_t h0 = 0x67452301;
        uint32_t h1 = 0xefcdab89;
        uint32_t h2 = 0x98badcfe;
        uint32_t h3 = 0x10325476;

        // Pre-processing: adding padding bits
        size_t newLen = ((length + 8) / 64 + 1) * 64;
        uint8_t* msg = new uint8_t[newLen]();
        memcpy(msg, data, length);
        msg[length] = 0x80;

        // Append original length in bits as 64-bit little-endian
        uint64_t bitsLen = length * 8;
        memcpy(msg + newLen - 8, &bitsLen, 8);

        // Process each 64-byte chunk
        for (size_t offset = 0; offset < newLen; offset += 64)
        {
            uint32_t* w = reinterpret_cast<uint32_t*>(msg + offset);

            uint32_t a = h0;
            uint32_t b = h1;
            uint32_t c = h2;
            uint32_t d = h3;

            for (uint32_t i = 0; i < 64; i++)
            {
                uint32_t f, g;
                if (i < 16)
                {
                    f = (b & c) | ((~b) & d);
                    g = i;
                }
                else if (i < 32)
                {
                    f = (d & b) | ((~d) & c);
                    g = (5 * i + 1) % 16;
                }
                else if (i < 48)
                {
                    f = b ^ c ^ d;
                    g = (3 * i + 5) % 16;
                }
                else
                {
                    f = c ^ (b | (~d));
                    g = (7 * i) % 16;
                }

                uint32_t temp = d;
                d = c;
                c = b;
                b = b + leftRotate(a + f + MD5_K[i] + w[g], MD5_S[i]);
                a = temp;
            }

            h0 += a;
            h1 += b;
            h2 += c;
            h3 += d;
        }

        delete[] msg;

        // Output hash in little-endian
        memcpy(hash, &h0, 4);
        memcpy(hash + 4, &h1, 4);
        memcpy(hash + 8, &h2, 4);
        memcpy(hash + 12, &h3, 4);
    }

    // ==================== AES-128 Implementation ====================

    // AES S-box
    static const uint8_t AES_SBOX[256] = {
        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
        0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
        0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
        0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
    };

    // AES inverse S-box
    static const uint8_t AES_INV_SBOX[256] = {
        0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
        0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
        0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
        0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
        0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
        0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
        0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
        0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
        0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
        0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
        0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
        0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
        0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
        0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
        0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
        0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
    };

    // AES Rcon
    static const uint8_t AES_RCON[11] = {
        0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
    };

    // Galois field multiplication
    static inline uint8_t gmul(uint8_t a, uint8_t b)
    {
        uint8_t p = 0;
        for (int i = 0; i < 8; i++)
        {
            if (b & 1)
                p ^= a;
            bool hi = a & 0x80;
            a <<= 1;
            if (hi)
                a ^= 0x1b;
            b >>= 1;
        }
        return p;
    }

    // Key expansion for AES-128
    static void aesKeyExpansion(const uint8_t* key, uint8_t* roundKeys)
    {
        memcpy(roundKeys, key, 16);

        uint8_t temp[4];
        int i = 16;
        int rconIdx = 1;

        while (i < 176)
        {
            memcpy(temp, roundKeys + i - 4, 4);

            if (i % 16 == 0)
            {
                // RotWord
                uint8_t t = temp[0];
                temp[0] = temp[1];
                temp[1] = temp[2];
                temp[2] = temp[3];
                temp[3] = t;

                // SubWord
                temp[0] = AES_SBOX[temp[0]];
                temp[1] = AES_SBOX[temp[1]];
                temp[2] = AES_SBOX[temp[2]];
                temp[3] = AES_SBOX[temp[3]];

                // XOR with Rcon
                temp[0] ^= AES_RCON[rconIdx++];
            }

            roundKeys[i] = roundKeys[i - 16] ^ temp[0];
            roundKeys[i + 1] = roundKeys[i - 15] ^ temp[1];
            roundKeys[i + 2] = roundKeys[i - 14] ^ temp[2];
            roundKeys[i + 3] = roundKeys[i - 13] ^ temp[3];
            i += 4;
        }
    }

    // AddRoundKey
    static inline void addRoundKey(uint8_t* state, const uint8_t* roundKey)
    {
        for (int i = 0; i < 16; i++)
            state[i] ^= roundKey[i];
    }

    // SubBytes
    static inline void subBytes(uint8_t* state)
    {
        for (int i = 0; i < 16; i++)
            state[i] = AES_SBOX[state[i]];
    }

    // InvSubBytes
    static inline void invSubBytes(uint8_t* state)
    {
        for (int i = 0; i < 16; i++)
            state[i] = AES_INV_SBOX[state[i]];
    }

    // ShiftRows
    static inline void shiftRows(uint8_t* state)
    {
        uint8_t temp;
        // Row 1: shift left by 1
        temp = state[1];
        state[1] = state[5];
        state[5] = state[9];
        state[9] = state[13];
        state[13] = temp;
        // Row 2: shift left by 2
        temp = state[2];
        state[2] = state[10];
        state[10] = temp;
        temp = state[6];
        state[6] = state[14];
        state[14] = temp;
        // Row 3: shift left by 3 (= right by 1)
        temp = state[15];
        state[15] = state[11];
        state[11] = state[7];
        state[7] = state[3];
        state[3] = temp;
    }

    // InvShiftRows
    static inline void invShiftRows(uint8_t* state)
    {
        uint8_t temp;
        // Row 1: shift right by 1
        temp = state[13];
        state[13] = state[9];
        state[9] = state[5];
        state[5] = state[1];
        state[1] = temp;
        // Row 2: shift right by 2
        temp = state[2];
        state[2] = state[10];
        state[10] = temp;
        temp = state[6];
        state[6] = state[14];
        state[14] = temp;
        // Row 3: shift right by 3 (= left by 1)
        temp = state[3];
        state[3] = state[7];
        state[7] = state[11];
        state[11] = state[15];
        state[15] = temp;
    }

    // MixColumns
    static inline void mixColumns(uint8_t* state)
    {
        for (int c = 0; c < 4; c++)
        {
            int i = c * 4;
            uint8_t a0 = state[i], a1 = state[i + 1], a2 = state[i + 2], a3 = state[i + 3];
            state[i] = gmul(a0, 2) ^ gmul(a1, 3) ^ a2 ^ a3;
            state[i + 1] = a0 ^ gmul(a1, 2) ^ gmul(a2, 3) ^ a3;
            state[i + 2] = a0 ^ a1 ^ gmul(a2, 2) ^ gmul(a3, 3);
            state[i + 3] = gmul(a0, 3) ^ a1 ^ a2 ^ gmul(a3, 2);
        }
    }

    // InvMixColumns
    static inline void invMixColumns(uint8_t* state)
    {
        for (int c = 0; c < 4; c++)
        {
            int i = c * 4;
            uint8_t a0 = state[i], a1 = state[i + 1], a2 = state[i + 2], a3 = state[i + 3];
            state[i] = gmul(a0, 14) ^ gmul(a1, 11) ^ gmul(a2, 13) ^ gmul(a3, 9);
            state[i + 1] = gmul(a0, 9) ^ gmul(a1, 14) ^ gmul(a2, 11) ^ gmul(a3, 13);
            state[i + 2] = gmul(a0, 13) ^ gmul(a1, 9) ^ gmul(a2, 14) ^ gmul(a3, 11);
            state[i + 3] = gmul(a0, 11) ^ gmul(a1, 13) ^ gmul(a2, 9) ^ gmul(a3, 14);
        }
    }

    void skylanderCrypto::aesEncryptBlock(const uint8_t* plaintext, const uint8_t* key, uint8_t* ciphertext)
    {
        uint8_t state[16];
        uint8_t roundKeys[176];

        memcpy(state, plaintext, 16);
        aesKeyExpansion(key, roundKeys);

        addRoundKey(state, roundKeys);

        for (int round = 1; round < 10; round++)
        {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, roundKeys + round * 16);
        }

        subBytes(state);
        shiftRows(state);
        addRoundKey(state, roundKeys + 160);

        memcpy(ciphertext, state, 16);
    }

    void skylanderCrypto::aesDecryptBlock(const uint8_t* ciphertext, const uint8_t* key, uint8_t* plaintext)
    {
        uint8_t state[16];
        uint8_t roundKeys[176];

        memcpy(state, ciphertext, 16);
        aesKeyExpansion(key, roundKeys);

        addRoundKey(state, roundKeys + 160);

        for (int round = 9; round > 0; round--)
        {
            invShiftRows(state);
            invSubBytes(state);
            addRoundKey(state, roundKeys + round * 16);
            invMixColumns(state);
        }

        invShiftRows(state);
        invSubBytes(state);
        addRoundKey(state, roundKeys);

        memcpy(plaintext, state, 16);
    }

    // ==================== Skylander-specific ====================

    void skylanderCrypto::generateBlockKey(const uint8_t* sector0, uint8_t blockIndex, uint8_t* key)
    {
        // Key = MD5(sector0[0:32] + blockIndex + ACTIVISION_COPYRIGHT)
        uint8_t buffer[32 + 1 + ACTIVISION_COPYRIGHT_LEN];
        memcpy(buffer, sector0, 32);
        buffer[32] = blockIndex;
        memcpy(buffer + 33, ACTIVISION_COPYRIGHT, ACTIVISION_COPYRIGHT_LEN);

        md5(buffer, sizeof(buffer), key);
    }

    bool skylanderCrypto::isSectorTrailer(uint8_t blockIndex)
    {
        // Sector trailers are at blocks 3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63
        return (blockIndex % 4) == 3;
    }

    bool skylanderCrypto::isEncryptedBlock(uint8_t blockIndex)
    {
        // Blocks 0-7 are not encrypted (sector 0 and 1)
        // Sector trailers are not encrypted
        // All other blocks (8+) are encrypted
        return (blockIndex >= 8) && !isSectorTrailer(blockIndex);
    }

    void skylanderCrypto::decryptSkylander(uint8_t* data)
    {
        uint8_t key[16];
        uint8_t decrypted[16];

        for (uint8_t block = 8; block < SKY_NUM_BLOCKS; block++)
        {
            if (!isEncryptedBlock(block))
                continue;

            generateBlockKey(data, block, key);
            aesDecryptBlock(data + block * SKY_BLOCK_SIZE, key, decrypted);
            memcpy(data + block * SKY_BLOCK_SIZE, decrypted, SKY_BLOCK_SIZE);
        }
    }

    void skylanderCrypto::encryptSkylander(uint8_t* data)
    {
        uint8_t key[16];
        uint8_t encrypted[16];

        for (uint8_t block = 8; block < SKY_NUM_BLOCKS; block++)
        {
            if (!isEncryptedBlock(block))
                continue;

            generateBlockKey(data, block, key);
            aesEncryptBlock(data + block * SKY_BLOCK_SIZE, key, encrypted);
            memcpy(data + block * SKY_BLOCK_SIZE, encrypted, SKY_BLOCK_SIZE);
        }
    }

    // ==================== Checksums ====================

    uint16_t skylanderCrypto::calcType0Checksum(const uint8_t* data)
    {
        // Type 0: CRC of first 0x1E bytes
        return crc16(data, 0x1E);
    }

    uint16_t skylanderCrypto::calcType1Checksum(const uint8_t* dataAreaHeader)
    {
        // Type 1: CRC of 16-byte header, with checksum field replaced by 0x0005
        uint8_t temp[16];
        memcpy(temp, dataAreaHeader, 16);
        temp[SKY_CRC1_OFFSET] = 0x05;
        temp[SKY_CRC1_OFFSET + 1] = 0x00;
        return crc16(temp, 16);
    }

    uint16_t skylanderCrypto::calcType2Checksum(const uint8_t* data, uint8_t areaBlock)
    {
        // Type 2: CRC of 3 blocks after header (0x30 bytes)
        // Skip sector trailer if present
        uint8_t buffer[0x30];
        size_t offset = 0;

        for (int i = 1; i <= 3; i++)
        {
            uint8_t block = areaBlock + i;
            if (!isSectorTrailer(block))
            {
                memcpy(buffer + offset, data + block * SKY_BLOCK_SIZE, SKY_BLOCK_SIZE);
                offset += SKY_BLOCK_SIZE;
            }
        }

        return crc16(buffer, offset);
    }

    uint16_t skylanderCrypto::calcType3Checksum(const uint8_t* data, uint8_t areaBlock)
    {
        // Type 3: CRC of remaining blocks in the data area + zero padding
        // This is more complex - covers blocks after Type 2 range
        uint8_t buffer[0x110]; // Enough for data + padding
        memset(buffer, 0, sizeof(buffer));
        size_t offset = 0;

        // For area 0: blocks 0x0C-0x0F (but 0x0F is trailer)
        // For area 1: blocks 0x28-0x2F (but 0x2B and 0x2F are trailers)
        for (int i = 4; i < 8; i++)
        {
            uint8_t block = areaBlock + i;
            if (block < SKY_NUM_BLOCKS && !isSectorTrailer(block))
            {
                memcpy(buffer + offset, data + block * SKY_BLOCK_SIZE, SKY_BLOCK_SIZE);
                offset += SKY_BLOCK_SIZE;
            }
        }

        // Pad with zeros to make it 0x0E blocks (224 bytes) total including data
        return crc16(buffer, 0x0E * SKY_BLOCK_SIZE);
    }

    uint8_t skylanderCrypto::getSequence(const uint8_t* data, uint8_t areaBlock)
    {
        return data[areaBlock * SKY_BLOCK_SIZE + SKY_SEQUENCE_OFFSET];
    }

    void skylanderCrypto::setSequence(uint8_t* data, uint8_t areaBlock, uint8_t sequence)
    {
        data[areaBlock * SKY_BLOCK_SIZE + SKY_SEQUENCE_OFFSET] = sequence;
    }

    uint8_t skylanderCrypto::getActiveArea(const uint8_t* data)
    {
        uint8_t seq0 = getSequence(data, SKY_AREA0_BLOCK);
        uint8_t seq1 = getSequence(data, SKY_AREA1_BLOCK);

        // Handle wraparound: if one is 0 and other is 255, 0 is newer
        if (seq0 == 0 && seq1 == 255)
            return SKY_AREA0_BLOCK;
        if (seq1 == 0 && seq0 == 255)
            return SKY_AREA1_BLOCK;

        return (seq1 > seq0) ? SKY_AREA1_BLOCK : SKY_AREA0_BLOCK;
    }

    void skylanderCrypto::updateAreaChecksums(uint8_t* data, uint8_t areaBlock)
    {
        uint8_t* header = data + areaBlock * SKY_BLOCK_SIZE;

        // Calculate in order: Type 3, Type 2, then Type 1
        uint16_t crc3 = calcType3Checksum(data, areaBlock);
        header[SKY_CRC3_OFFSET] = crc3 & 0xFF;
        header[SKY_CRC3_OFFSET + 1] = (crc3 >> 8) & 0xFF;

        uint16_t crc2 = calcType2Checksum(data, areaBlock);
        header[SKY_CRC2_OFFSET] = crc2 & 0xFF;
        header[SKY_CRC2_OFFSET + 1] = (crc2 >> 8) & 0xFF;

        uint16_t crc1 = calcType1Checksum(header);
        header[SKY_CRC1_OFFSET] = crc1 & 0xFF;
        header[SKY_CRC1_OFFSET + 1] = (crc1 >> 8) & 0xFF;
    }
}
