/*
 * sky-reset - Skylander Factory Reset Tool
 *
 * Resets a Skylander figure to factory blank state by:
 * - Zeroing experience, money, skills, hats, hero points, etc.
 * - Updating sequence counters and checksums
 * - Properly encrypting the data
 *
 * The Skylander's identity (character type) is preserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fstream>
#include <string>
#include <iostream>

#include "cxxopts.hpp"
#include "appError.hpp"
#include "acr122u.hpp"
#include "skylanderNFC.hpp"
#include "skylanderCrypto.hpp"
#include "skylanderDB.hpp"

using namespace xk;

// Data area field offsets (relative to data area header block)
#define FIELD_XP_OFFSET      0x00  // 3 bytes
#define FIELD_MONEY_OFFSET   0x03  // 2 bytes
#define FIELD_CUMUL_OFFSET   0x05  // 4 bytes (cumulative time)
#define FIELD_SEQUENCE       0x09  // 1 byte
#define FIELD_CRC3_OFFSET    0x0A  // 2 bytes
#define FIELD_CRC2_OFFSET    0x0C  // 2 bytes
#define FIELD_CRC1_OFFSET    0x0E  // 2 bytes

// Block 0x09/0x25 offsets (skills, platform, hat)
#define FIELD_SKILLS_OFFSET  0x00  // 2 bytes
#define FIELD_PLATFORM_OFFSET 0x02 // 1 byte
#define FIELD_HAT_OFFSET     0x03  // 2 bytes
#define FIELD_UNKNOWN1       0x05  // 2 bytes
#define FIELD_UNKNOWN2       0x07  // 2 bytes

// Block 0x0D/0x29 offsets (hero points, heroic challenges)
#define FIELD_HERO_CHAL_OFFSET 0x06 // 4 bytes (heroic challenges bitmask)
#define FIELD_HERO_PTS_OFFSET  0x0A // 1 byte (hero points)

// Reset a single data area (blocks starting at areaBlock)
void resetDataArea(uint8_t* data, uint8_t areaBlock, uint8_t newSequence)
{
    // Get pointers to relevant blocks
    uint8_t* header = data + areaBlock * SKY_BLOCK_SIZE;        // Block 0x08 or 0x24
    uint8_t* skills = data + (areaBlock + 1) * SKY_BLOCK_SIZE;  // Block 0x09 or 0x25
    uint8_t* name1 = data + (areaBlock + 2) * SKY_BLOCK_SIZE;   // Block 0x0A or 0x26
    uint8_t* name2 = data + (areaBlock + 4) * SKY_BLOCK_SIZE;   // Block 0x0C or 0x28
    uint8_t* heroData = data + (areaBlock + 5) * SKY_BLOCK_SIZE; // Block 0x0D or 0x29

    // Reset header block: XP, Money, Cumulative time
    memset(header + FIELD_XP_OFFSET, 0, 3);     // XP = 0
    memset(header + FIELD_MONEY_OFFSET, 0, 2);  // Money = 0
    memset(header + FIELD_CUMUL_OFFSET, 0, 4);  // Cumulative time = 0

    // Set sequence counter
    header[FIELD_SEQUENCE] = newSequence;

    // Reset skills block: skills, hat, etc.
    memset(skills + FIELD_SKILLS_OFFSET, 0, 2);  // Skills = 0
    // Keep platform byte as-is (it identifies which platform the figure was used on)
    memset(skills + FIELD_HAT_OFFSET, 0, 2);     // Hat = 0 (no hat)
    memset(skills + FIELD_UNKNOWN1, 0, 2);
    memset(skills + FIELD_UNKNOWN2, 0, 2);

    // Clear name blocks (set to blank Unicode spaces or zeros)
    memset(name1, 0, SKY_BLOCK_SIZE);
    memset(name2, 0, SKY_BLOCK_SIZE);

    // Reset hero data: heroic challenges and hero points
    memset(heroData + FIELD_HERO_CHAL_OFFSET, 0, 4);  // Heroic challenges = none
    heroData[FIELD_HERO_PTS_OFFSET] = 0;              // Hero points = 0

    // Clear any remaining data blocks in this area
    for (int i = 6; i < 8; i++)
    {
        uint8_t blockIdx = areaBlock + i;
        if (!skylanderCrypto::isSectorTrailer(blockIdx) && blockIdx < SKY_NUM_BLOCKS)
        {
            memset(data + blockIdx * SKY_BLOCK_SIZE, 0, SKY_BLOCK_SIZE);
        }
    }
}

int main(int argc, char** argv)
{
    std::string inputFileName = "";
    std::string outputFileName = "";
    bool useNFC = true;
    bool dryRun = false;

    // Setup command line options
    cxxopts::Options app_options("sky-reset", "Skylander Factory Reset Tool - Reset a Skylander to blank state");
    app_options.add_options()
        ("f,file", "Input BIN file (if not using NFC reader)", cxxopts::value<std::string>())
        ("o,output", "Output BIN file (default: overwrite input or write to card)", cxxopts::value<std::string>())
        ("n,dry-run", "Dry run - don't write changes, just show what would happen")
        ("h,help", "Show help");

    // Parse options
    cxxopts::ParseResult app_params;
    try
    {
        app_params = app_options.parse(argc, argv);
    }
    catch (const cxxopts::exceptions::exception& e)
    {
        std::cout << app_options.help() << std::endl;
        return -1;
    }

    if (app_params.count("help"))
    {
        std::cout << app_options.help() << std::endl;
        return 0;
    }

    if (app_params.count("file"))
    {
        inputFileName = app_params["file"].as<std::string>();
        useNFC = false;
    }

    if (app_params.count("output"))
    {
        outputFileName = app_params["output"].as<std::string>();
    }

    if (app_params.count("dry-run"))
    {
        dryRun = true;
    }

    // Data buffer
    acr122u::NFC_Sector sectors[NFC_SECTORS_PER_CARD];
    uint8_t* data = reinterpret_cast<uint8_t*>(sectors);

    std::string readerName;

    if (useNFC)
    {
        // Read from NFC card
        std::cout << "STEP 1:\tReading Skylander from NFC Reader" << std::endl;

        try
        {
            acr122u nfc;
            readerName = nfc.findAndConnect();
            std::cout << "\tFound: " << readerName << std::endl;
            std::cout << "\tConnected." << std::endl;

            // Authenticate and read sector 0
            uint8_t sky_key_a[] = SKYLANDER_BLOCK_0_KEY_A;
            uint32_t retval = nfc.sectorAuthKeyA(0, sky_key_a);
            if (retval != NFC_OK)
            {
                std::cout << "** ERROR: Failed to authenticate sector 0. Is this a Skylander?" << std::endl;
                return -1;
            }

            retval = nfc.sectorRead(0, sectors[0]);
            if (retval != NFC_OK)
            {
                std::cout << "** ERROR: Failed to read sector 0!" << std::endl;
                return -1;
            }

            // Get UID
            uint8_t uid[NFC_UID_SIZE];
            memcpy(uid, sectors[0].blk0, NFC_UID_SIZE);
            std::cout << "\tSkylander UID: " << skylanderNFC::toHexStr(uid, NFC_UID_SIZE) << std::endl;

            // Read all sectors
            std::cout << "\tReading all sectors: ";
            for (uint8_t sec = 1; sec < NFC_SECTORS_PER_CARD; sec++)
            {
                uint64_t sector_key = skylanderNFC::makeSkyKey(sec, uid);
                uint8_t key[NFC_KEY_SIZE];
                memcpy(key, &sector_key, NFC_KEY_SIZE);

                retval = nfc.sectorAuthKeyA(sec, key);
                if (retval == NFC_OK)
                    retval = nfc.sectorRead(sec, sectors[sec]);

                if (retval == NFC_OK)
                    std::cout << "." << std::flush;
                else
                    std::cout << "X" << std::flush;
            }
            std::cout << " Done." << std::endl;

            nfc.deviceDisconnect();
        }
        catch (appError* e)
        {
            std::cout << "Error: " << e->what() << std::endl;
            delete e;
            return -1;
        }
    }
    else
    {
        // Read from file
        std::cout << "STEP 1:\tReading Skylander from file \"" << inputFileName << "\"" << std::endl;

        std::ifstream infile(inputFileName, std::ios::binary);
        if (!infile.is_open())
        {
            std::cout << "** ERROR: Could not open file: " << inputFileName << std::endl;
            return -1;
        }

        infile.seekg(0, std::ios::end);
        size_t fileSize = infile.tellg();
        infile.seekg(0, std::ios::beg);

        if (fileSize != NFC_CARD_SIZE)
        {
            std::cout << "** ERROR: Invalid file size. Expected " << NFC_CARD_SIZE << " bytes, got " << fileSize << std::endl;
            return -1;
        }

        infile.read(reinterpret_cast<char*>(data), NFC_CARD_SIZE);
        infile.close();

        uint8_t uid[NFC_UID_SIZE];
        memcpy(uid, data, NFC_UID_SIZE);
        std::cout << "\tSkylander UID: " << skylanderNFC::toHexStr(uid, NFC_UID_SIZE) << std::endl;
    }

    // Identify Skylander (character ID is not encrypted)
    uint16_t charId = skylanderDB::getCharacterId(data);
    const SkylanderInfo* info = skylanderDB::getInfo(charId);
    if (info)
    {
        std::cout << "\tSkylander: " << info->name << " (" << skylanderDB::getTypeString(info->type) << ", " << info->element << ")" << std::endl;
    }
    else
    {
        std::cout << "\tSkylander: Unknown (ID: 0x" << std::hex << charId << std::dec << ")" << std::endl;
    }

    // Decrypt the data
    std::cout << "STEP 2:\tDecrypting Skylander data" << std::endl;
    skylanderCrypto::decryptSkylander(data);
    std::cout << "\tDecryption complete." << std::endl;

    // Show current state
    uint8_t activeArea = skylanderCrypto::getActiveArea(data);
    uint8_t* activeHeader = data + activeArea * SKY_BLOCK_SIZE;

    uint32_t currentXP = activeHeader[0] | (activeHeader[1] << 8) | (activeHeader[2] << 16);
    uint16_t currentMoney = activeHeader[3] | (activeHeader[4] << 8);
    uint8_t seq0 = skylanderCrypto::getSequence(data, SKY_AREA0_BLOCK);
    uint8_t seq1 = skylanderCrypto::getSequence(data, SKY_AREA1_BLOCK);

    std::cout << "\tActive Area: " << (activeArea == SKY_AREA0_BLOCK ? "0" : "1") << std::endl;
    std::cout << "\tCurrent XP: " << currentXP << std::endl;
    std::cout << "\tCurrent Money: " << currentMoney << std::endl;
    std::cout << "\tSequence counters: Area0=" << (int)seq0 << ", Area1=" << (int)seq1 << std::endl;

    // Perform reset
    std::cout << "STEP 3:\tResetting Skylander data" << std::endl;

    // Determine new sequence numbers
    uint8_t maxSeq = (seq0 > seq1) ? seq0 : seq1;
    uint8_t newSeq = maxSeq + 1;

    // Reset both data areas
    resetDataArea(data, SKY_AREA0_BLOCK, newSeq);
    resetDataArea(data, SKY_AREA1_BLOCK, newSeq);

    std::cout << "\tCleared XP, Money, Skills, Hats, Hero data" << std::endl;
    std::cout << "\tNew sequence counter: " << (int)newSeq << std::endl;

    // Update checksums
    std::cout << "STEP 4:\tRecalculating checksums" << std::endl;
    skylanderCrypto::updateAreaChecksums(data, SKY_AREA0_BLOCK);
    skylanderCrypto::updateAreaChecksums(data, SKY_AREA1_BLOCK);
    std::cout << "\tChecksums updated." << std::endl;

    // Re-encrypt
    std::cout << "STEP 5:\tRe-encrypting Skylander data" << std::endl;
    skylanderCrypto::encryptSkylander(data);
    std::cout << "\tEncryption complete." << std::endl;

    if (dryRun)
    {
        std::cout << "\n** DRY RUN - No changes written **" << std::endl;
        return 0;
    }

    // Write back
    if (useNFC)
    {
        std::cout << "STEP 6:\tWriting reset Skylander to NFC card" << std::endl;

        try
        {
            acr122u nfc;
            nfc.findAndConnect();

            uint8_t uid[NFC_UID_SIZE];
            memcpy(uid, data, NFC_UID_SIZE);

            std::cout << "\tWriting sectors: ";

            for (uint8_t sec = 1; sec < NFC_SECTORS_PER_CARD; sec++)
            {
                uint64_t sector_key = skylanderNFC::makeSkyKey(sec, uid);
                uint8_t key[NFC_KEY_SIZE];
                memcpy(key, &sector_key, NFC_KEY_SIZE);

                uint32_t retval = nfc.sectorAuthKeyA(sec, key);
                if (retval == NFC_OK)
                {
                    // Write first 3 blocks (skip sector trailer)
                    retval = nfc.sectorWrite(sec, sectors[sec], 0, 3);
                }

                if (retval == NFC_OK)
                    std::cout << "." << std::flush;
                else
                    std::cout << "X" << std::flush;
            }
            std::cout << " Done." << std::endl;

            // Verify
            std::cout << "STEP 7:\tVerifying written data" << std::endl;
            acr122u::NFC_Sector verifySectors[NFC_SECTORS_PER_CARD];
            bool verifyOk = true;

            for (uint8_t sec = 1; sec < NFC_SECTORS_PER_CARD; sec++)
            {
                uint64_t sector_key = skylanderNFC::makeSkyKey(sec, uid);
                uint8_t key[NFC_KEY_SIZE];
                memcpy(key, &sector_key, NFC_KEY_SIZE);

                uint32_t retval = nfc.sectorAuthKeyA(sec, key);
                if (retval == NFC_OK)
                    retval = nfc.sectorRead(sec, verifySectors[sec]);

                if (retval == NFC_OK)
                {
                    // Compare first 3 blocks
                    if (memcmp(&sectors[sec], &verifySectors[sec], 48) != 0)
                    {
                        verifyOk = false;
                        std::cout << "X" << std::flush;
                    }
                    else
                    {
                        std::cout << "." << std::flush;
                    }
                }
                else
                {
                    std::cout << "X" << std::flush;
                    verifyOk = false;
                }
            }

            nfc.deviceDisconnect();

            if (verifyOk)
                std::cout << "\n\nReset Complete - Verification PASSED!" << std::endl;
            else
                std::cout << "\n\n** WARNING: Verification detected differences!" << std::endl;
        }
        catch (appError* e)
        {
            std::cout << "Error: " << e->what() << std::endl;
            delete e;
            return -1;
        }
    }
    else
    {
        // Write to file
        std::string outFile = outputFileName.empty() ? inputFileName : outputFileName;
        std::cout << "STEP 6:\tWriting reset Skylander to file \"" << outFile << "\"" << std::endl;

        std::ofstream outfile(outFile, std::ios::binary);
        if (!outfile.is_open())
        {
            std::cout << "** ERROR: Could not open output file: " << outFile << std::endl;
            return -1;
        }

        outfile.write(reinterpret_cast<char*>(data), NFC_CARD_SIZE);
        outfile.close();

        std::cout << "\tDone." << std::endl;
        std::cout << "\nReset Complete!" << std::endl;
    }

    return 0;
}
