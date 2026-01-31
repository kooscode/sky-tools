/*
 * Skylander Encryption and Checksum Library
 *
 * Implements:
 * - AES-128-ECB encryption/decryption with MD5 key derivation
 * - CRC-16/CCITT-FALSE checksums (Types 0, 1, 2, 3)
 *
 * Based on reverse engineering work from:
 * - https://marijnkneppers.dev/posts/reverse-engineering-skylanders-toys-to-life-mechanics/
 * - https://github.com/silicontrip/SkyReader
 * - https://nfc.toys/interop-sky.html
 */

#ifndef SKYLANDER_CRYPTO_HPP
#define SKYLANDER_CRYPTO_HPP

#include <stdint.h>
#include <string.h>

namespace xk
{
    // Block and sector constants
    #define SKY_BLOCK_SIZE 16
    #define SKY_SECTOR_SIZE 64
    #define SKY_CARD_SIZE 1024
    #define SKY_NUM_BLOCKS 64
    #define SKY_NUM_SECTORS 16

    // Data area block offsets
    #define SKY_AREA0_BLOCK 0x08
    #define SKY_AREA1_BLOCK 0x24

    // Data field offsets within data area header block
    #define SKY_XP_OFFSET 0x00
    #define SKY_XP_SIZE 3
    #define SKY_MONEY_OFFSET 0x03
    #define SKY_MONEY_SIZE 2
    #define SKY_SEQUENCE_OFFSET 0x09
    #define SKY_CRC3_OFFSET 0x0A
    #define SKY_CRC2_OFFSET 0x0C
    #define SKY_CRC1_OFFSET 0x0E

    // Activision copyright string used in key derivation (53 bytes)
    static const char ACTIVISION_COPYRIGHT[] = " Copyright (C) 2010 Activision. All Rights Reserved.";
    #define ACTIVISION_COPYRIGHT_LEN 53

    class skylanderCrypto
    {
    public:
        skylanderCrypto() = delete;

        // ==================== CRC-16/CCITT-FALSE ====================

        // Calculate CRC-16/CCITT-FALSE checksum
        // Polynomial: 0x1021, Init: 0xFFFF, No reflect, No XOR out
        static uint16_t crc16(const uint8_t* data, size_t length, uint16_t init = 0xFFFF);

        // ==================== MD5 Hash ====================

        // Calculate MD5 hash (returns 16 bytes)
        static void md5(const uint8_t* data, size_t length, uint8_t* hash);

        // ==================== AES-128-ECB ====================

        // Encrypt a single 16-byte block using AES-128-ECB
        static void aesEncryptBlock(const uint8_t* plaintext, const uint8_t* key, uint8_t* ciphertext);

        // Decrypt a single 16-byte block using AES-128-ECB
        static void aesDecryptBlock(const uint8_t* ciphertext, const uint8_t* key, uint8_t* plaintext);

        // ==================== Skylander-specific ====================

        // Generate AES key for a specific block
        // key = MD5(sector0[0:32] + blockIndex + ACTIVISION_COPYRIGHT)
        static void generateBlockKey(const uint8_t* sector0, uint8_t blockIndex, uint8_t* key);

        // Decrypt all encrypted blocks in a skylander dump
        // Blocks 0x08+ are encrypted (except sector trailers at blocks 3,7,11,15,...)
        static void decryptSkylander(uint8_t* data);

        // Encrypt all data blocks in a skylander dump
        static void encryptSkylander(uint8_t* data);

        // Check if a block is a sector trailer (contains keys, not encrypted)
        static bool isSectorTrailer(uint8_t blockIndex);

        // Check if a block should be encrypted
        static bool isEncryptedBlock(uint8_t blockIndex);

        // ==================== Checksums ====================

        // Calculate Type 0 checksum (sector 0 validation)
        // Covers first 0x1E bytes (blocks 0-1), stored at block 1 offset 0x0E
        static uint16_t calcType0Checksum(const uint8_t* data);

        // Calculate Type 1 checksum (data area header)
        // Covers 16 bytes of data area header, stored at offset 0x0E
        // Must replace checksum bytes with 0x0005 before calculation
        static uint16_t calcType1Checksum(const uint8_t* dataAreaHeader);

        // Calculate Type 2 checksum (first 4 data blocks after header)
        // Covers 0x30 bytes (3 blocks, excluding sector trailer), stored at offset 0x0C
        static uint16_t calcType2Checksum(const uint8_t* data, uint8_t areaBlock);

        // Calculate Type 3 checksum (next data blocks + zero padding)
        // Stored at offset 0x0A
        static uint16_t calcType3Checksum(const uint8_t* data, uint8_t areaBlock);

        // Get sequence counter for a data area
        static uint8_t getSequence(const uint8_t* data, uint8_t areaBlock);

        // Set sequence counter for a data area
        static void setSequence(uint8_t* data, uint8_t areaBlock, uint8_t sequence);

        // Determine which data area is active (returns SKY_AREA0_BLOCK or SKY_AREA1_BLOCK)
        static uint8_t getActiveArea(const uint8_t* data);

        // Update all checksums for a data area (call after modifying data)
        // Order: Type 3, Type 2, then Type 1
        static void updateAreaChecksums(uint8_t* data, uint8_t areaBlock);
    };
}

#endif /* SKYLANDER_CRYPTO_HPP */
