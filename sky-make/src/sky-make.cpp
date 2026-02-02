/*
 * sky-make - Create new Skylanders from scratch
 * Interactive tool to build custom Skylander NFC data
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <random>

#include "cxxopts.hpp"
#include "appError.hpp"
#include "acr122u.hpp"
#include "skylanderNFC.hpp"
#include "skylanderDB.hpp"
#include "skylanderCrypto.hpp"
#include "hatDB.hpp"

// Skylander data structure configuration
struct SkylanderConfig
{
    uint16_t characterId = 0;
    uint16_t variantId = 0;
    uint32_t xp = 0;
    uint16_t money = 0;
    uint16_t skills = 0;
    uint8_t platform = 0x00;  // 0x00 = PC/other
    uint16_t hat = 0;
    uint8_t heroPoints = 0;
    uint32_t heroicChallenges = 0;
    std::string customName;
    uint8_t uid[4] = {0};
    bool hasUID = false;
};

// Forward declarations
void printBanner();
bool interactiveMode(SkylanderConfig& config);
bool buildSkylander(const SkylanderConfig& config, uint8_t* data);
void initSector0(uint8_t* data, const uint8_t* uid, uint16_t charId, uint16_t variantId);
void initSectorTrailers(uint8_t* data, const uint8_t* uid);
void initDataArea(uint8_t* data, uint8_t areaBlock, const SkylanderConfig& config);
void generateRandomUID(uint8_t* uid);
bool writeToNFC(const uint8_t* data);
bool writeToFile(const uint8_t* data, const std::string& filename);
int selectFromMenu(const std::string& prompt, const std::vector<std::string>& options);
uint32_t getNumberInput(const std::string& prompt, uint32_t min, uint32_t max, uint32_t defaultVal);
std::string getStringInput(const std::string& prompt, size_t maxLen);
void listCharacters(xk::SkylanderGame game);

int main(int argc, char** argv)
{
    SkylanderConfig config;
    std::string outputFile;
    bool interactive = true;

    // Setup command line options
    cxxopts::Options app_options("sky-make", "Skylander Creation Tool - Create new Skylanders from scratch");
    app_options.add_options()
        ("i,interactive", "Interactive mode (default)")
        ("o,output", "Output to BIN file instead of NFC card", cxxopts::value<std::string>())
        ("c,character", "Character ID or name", cxxopts::value<std::string>())
        ("x,xp", "XP value (0-16777215)", cxxopts::value<uint32_t>())
        ("m,money", "Money value (0-65535)", cxxopts::value<uint16_t>())
        ("a,hat", "Hat ID (0 = no hat)", cxxopts::value<uint16_t>())
        ("p,points", "Hero points (0-255)", cxxopts::value<uint8_t>())
        ("n,name", "Custom name (max 16 chars)", cxxopts::value<std::string>())
        ("l,list", "List all available characters")
        ("h,help", "Show help");

    // Parse options
    cxxopts::ParseResult params;
    try
    {
        params = app_options.parse(argc, argv);
    }
    catch (const cxxopts::exceptions::exception& e)
    {
        std::cout << app_options.help() << std::endl;
        return -1;
    }

    if (params.count("help"))
    {
        std::cout << app_options.help() << std::endl;
        return 0;
    }

    if (params.count("list"))
    {
        listCharacters(xk::SKY_GAME_UNKNOWN);
        return 0;
    }

    // Check for output file
    if (params.count("output"))
    {
        outputFile = params["output"].as<std::string>();
    }

    // Check for command-line character specification
    if (params.count("character"))
    {
        std::string charSpec = params["character"].as<std::string>();

        // Try parsing as ID first
        try
        {
            config.characterId = static_cast<uint16_t>(std::stoi(charSpec));
        }
        catch (...)
        {
            // Try finding by name
            const xk::SkylanderInfo* info = xk::skylanderDB::findByName(charSpec);
            if (info)
            {
                config.characterId = info->id;
            }
            else
            {
                std::cerr << "Unknown character: " << charSpec << std::endl;
                return -1;
            }
        }
        interactive = false;
    }

    // Apply other command-line options
    if (params.count("xp"))
        config.xp = std::min(params["xp"].as<uint32_t>(), 0xFFFFFFu);
    if (params.count("money"))
        config.money = params["money"].as<uint16_t>();
    if (params.count("hat"))
        config.hat = params["hat"].as<uint16_t>();
    if (params.count("points"))
        config.heroPoints = params["points"].as<uint8_t>();
    if (params.count("name"))
        config.customName = params["name"].as<std::string>().substr(0, 16);

    // Run interactive mode if no character specified
    if (interactive)
    {
        printBanner();
        if (!interactiveMode(config))
        {
            std::cout << "Cancelled." << std::endl;
            return 0;
        }
    }

    // Validate character
    const xk::SkylanderInfo* info = xk::skylanderDB::getInfo(config.characterId);
    if (!info)
    {
        std::cerr << "Invalid character ID: " << config.characterId << std::endl;
        return -1;
    }

    // Build Skylander data
    uint8_t data[1024];
    memset(data, 0, sizeof(data));

    if (!buildSkylander(config, data))
    {
        std::cerr << "Failed to build Skylander data" << std::endl;
        return -1;
    }

    // Output
    if (!outputFile.empty())
    {
        if (writeToFile(data, outputFile))
        {
            std::cout << "Skylander written to: " << outputFile << std::endl;
            return 0;
        }
        else
        {
            std::cerr << "Failed to write file: " << outputFile << std::endl;
            return -1;
        }
    }
    else
    {
        if (writeToNFC(data))
        {
            std::cout << "Skylander written to NFC card successfully!" << std::endl;
            return 0;
        }
        else
        {
            std::cerr << "Failed to write to NFC card" << std::endl;
            return -1;
        }
    }
}

void printBanner()
{
    std::cout << "========================================" << std::endl;
    std::cout << "       SKY-MAKE - Skylander Creator" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
}

bool interactiveMode(SkylanderConfig& config)
{
    // Step 1: Select Game
    std::cout << "STEP 1: Select Game/Generation" << std::endl;
    std::vector<std::string> gameOptions = {
        "All Games",
        "Spyro's Adventure",
        "Giants",
        "Swap Force",
        "Trap Team",
        "SuperChargers",
        "Imaginators"
    };

    int gameChoice = selectFromMenu("Select game", gameOptions);
    if (gameChoice < 0) return false;

    xk::SkylanderGame filterGame = xk::SKY_GAME_UNKNOWN;
    switch (gameChoice)
    {
        case 1: filterGame = xk::SKY_GAME_SPYROS_ADVENTURE; break;
        case 2: filterGame = xk::SKY_GAME_GIANTS; break;
        case 3: filterGame = xk::SKY_GAME_SWAP_FORCE; break;
        case 4: filterGame = xk::SKY_GAME_TRAP_TEAM; break;
        case 5: filterGame = xk::SKY_GAME_SUPERCHARGERS; break;
        case 6: filterGame = xk::SKY_GAME_IMAGINATORS; break;
    }
    std::cout << std::endl;

    // Step 2: Select Character
    std::cout << "STEP 2: Select Character" << std::endl;

    // Build filtered character list
    std::vector<const xk::SkylanderInfo*> characters;
    const xk::SkylanderInfo* allInfo = xk::skylanderDB::getDatabase();
    size_t count = xk::skylanderDB::getDatabaseSize();

    for (size_t i = 0; i < count; i++)
    {
        if (filterGame == xk::SKY_GAME_UNKNOWN || allInfo[i].game == filterGame)
        {
            characters.push_back(&allInfo[i]);
        }
    }

    if (characters.empty())
    {
        std::cout << "No characters found for selected game." << std::endl;
        return false;
    }

    // Display paginated character list
    const size_t pageSize = 20;
    size_t page = 0;
    size_t totalPages = (characters.size() + pageSize - 1) / pageSize;

    while (true)
    {
        std::cout << "\nCharacters (Page " << (page + 1) << "/" << totalPages << "):" << std::endl;

        size_t start = page * pageSize;
        size_t end = std::min(start + pageSize, characters.size());

        for (size_t i = start; i < end; i++)
        {
            const xk::SkylanderInfo* c = characters[i];
            std::cout << "  " << (i + 1) << ". " << c->name
                      << " (" << xk::skylanderDB::getElementString(c->element) << ")"
                      << " - " << xk::skylanderDB::getTypeString(c->type)
                      << std::endl;
        }

        std::cout << "\nEnter number (1-" << characters.size() << "), "
                  << "'n' for next page, 'p' for previous, 'q' to quit: ";

        std::string input;
        std::getline(std::cin, input);

        if (input == "q" || input == "Q") return false;
        if (input == "n" || input == "N")
        {
            if (page < totalPages - 1) page++;
            continue;
        }
        if (input == "p" || input == "P")
        {
            if (page > 0) page--;
            continue;
        }

        try
        {
            size_t choice = std::stoul(input);
            if (choice >= 1 && choice <= characters.size())
            {
                config.characterId = characters[choice - 1]->id;
                std::cout << "Selected: " << characters[choice - 1]->name << std::endl;
                break;
            }
        }
        catch (...) {}

        std::cout << "Invalid selection." << std::endl;
    }
    std::cout << std::endl;

    // Get character info for type-specific handling
    const xk::SkylanderInfo* selectedInfo = xk::skylanderDB::getInfo(config.characterId);

    // Step 3: Configure Stats (skip for traps and crystals)
    if (selectedInfo &&
        selectedInfo->type != xk::SKY_TYPE_TRAP &&
        selectedInfo->type != xk::SKY_TYPE_CREATION_CRYSTAL &&
        selectedInfo->type != xk::SKY_TYPE_VEHICLE)
    {
        std::cout << "STEP 3: Configure Stats" << std::endl;
        config.xp = getNumberInput("  XP (0-16777215)", 0, 0xFFFFFF, 0);
        config.money = static_cast<uint16_t>(getNumberInput("  Money (0-65535)", 0, 65535, 0));
        config.heroPoints = static_cast<uint8_t>(getNumberInput("  Hero Points (0-255)", 0, 255, 0));
        std::cout << std::endl;

        // Step 4: Select Hat
        std::cout << "STEP 4: Select Hat (optional)" << std::endl;
        std::cout << "  0. No Hat" << std::endl;
        std::cout << "  Enter hat ID (0-" << xk::hatDB::getHatCount() - 1 << ") or 'l' to list hats: ";

        while (true)
        {
            std::string input;
            std::getline(std::cin, input);

            if (input == "l" || input == "L")
            {
                const xk::HatInfo* hats = xk::hatDB::getHatList();
                size_t hatCount = xk::hatDB::getHatCount();
                for (size_t i = 0; i < hatCount; i++)
                {
                    std::cout << "  " << hats[i].id << ". " << hats[i].name << std::endl;
                }
                std::cout << "  Enter hat ID: ";
                continue;
            }

            try
            {
                config.hat = static_cast<uint16_t>(std::stoul(input));
                break;
            }
            catch (...)
            {
                std::cout << "  Invalid input. Enter hat ID: ";
            }
        }
        std::cout << std::endl;

        // Step 5: Custom Name
        std::cout << "STEP 5: Custom Name (optional)" << std::endl;
        config.customName = getStringInput("  Enter name (max 16 chars, empty to skip)", 16);
        std::cout << std::endl;
    }
    else
    {
        std::cout << "Note: Traps, Creation Crystals, and Vehicles don't have stats." << std::endl;
        std::cout << std::endl;
    }

    // Step 6: Confirm
    std::cout << "STEP 6: Review and Confirm" << std::endl;
    std::cout << "  Character:    " << selectedInfo->name << std::endl;
    std::cout << "  Type:         " << xk::skylanderDB::getTypeString(selectedInfo->type) << std::endl;
    std::cout << "  Element:      " << xk::skylanderDB::getElementString(selectedInfo->element) << std::endl;
    std::cout << "  Game:         " << xk::skylanderDB::getGameString(selectedInfo->game) << std::endl;
    if (selectedInfo->type != xk::SKY_TYPE_TRAP &&
        selectedInfo->type != xk::SKY_TYPE_CREATION_CRYSTAL &&
        selectedInfo->type != xk::SKY_TYPE_VEHICLE)
    {
        std::cout << "  XP:           " << config.xp << std::endl;
        std::cout << "  Money:        " << config.money << std::endl;
        std::cout << "  Hero Points:  " << (int)config.heroPoints << std::endl;
        std::cout << "  Hat:          " << xk::hatDB::getHatName(config.hat) << std::endl;
        if (!config.customName.empty())
            std::cout << "  Custom Name:  " << config.customName << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Create this Skylander? (y/n): ";
    std::string confirm;
    std::getline(std::cin, confirm);

    return (confirm == "y" || confirm == "Y" || confirm == "yes" || confirm == "Yes");
}

int selectFromMenu(const std::string& prompt, const std::vector<std::string>& options)
{
    for (size_t i = 0; i < options.size(); i++)
    {
        std::cout << "  " << i << ". " << options[i] << std::endl;
    }

    std::cout << prompt << " (0-" << (options.size() - 1) << "): ";
    std::string input;
    std::getline(std::cin, input);

    try
    {
        int choice = std::stoi(input);
        if (choice >= 0 && choice < static_cast<int>(options.size()))
            return choice;
    }
    catch (...) {}

    return -1;
}

uint32_t getNumberInput(const std::string& prompt, uint32_t min, uint32_t max, uint32_t defaultVal)
{
    std::cout << prompt << " [" << defaultVal << "]: ";
    std::string input;
    std::getline(std::cin, input);

    if (input.empty())
        return defaultVal;

    try
    {
        uint32_t val = std::stoul(input);
        return std::min(std::max(val, min), max);
    }
    catch (...)
    {
        return defaultVal;
    }
}

std::string getStringInput(const std::string& prompt, size_t maxLen)
{
    std::cout << prompt << ": ";
    std::string input;
    std::getline(std::cin, input);

    if (input.length() > maxLen)
        input = input.substr(0, maxLen);

    return input;
}

void listCharacters(xk::SkylanderGame game)
{
    const xk::SkylanderInfo* allInfo = xk::skylanderDB::getDatabase();
    size_t count = xk::skylanderDB::getDatabaseSize();

    std::cout << "Available Skylanders:" << std::endl;
    std::cout << "---------------------" << std::endl;

    for (size_t i = 0; i < count; i++)
    {
        if (game == xk::SKY_GAME_UNKNOWN || allInfo[i].game == game)
        {
            std::cout << allInfo[i].id << ": " << allInfo[i].name
                      << " (" << xk::skylanderDB::getElementString(allInfo[i].element) << ")"
                      << " - " << xk::skylanderDB::getTypeString(allInfo[i].type)
                      << " [" << xk::skylanderDB::getGameString(allInfo[i].game) << "]"
                      << std::endl;
        }
    }
}

void generateRandomUID(uint8_t* uid)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    for (int i = 0; i < 4; i++)
    {
        uid[i] = static_cast<uint8_t>(dis(gen));
    }
}

void initSector0(uint8_t* data, const uint8_t* uid, uint16_t charId, uint16_t variantId)
{
    // Block 0: UID and manufacturer data
    memcpy(data, uid, 4);
    data[4] = uid[0] ^ uid[1] ^ uid[2] ^ uid[3];  // BCC
    data[5] = 0x08;  // SAK
    data[6] = 0x04;  // ATQA

    // Standard Skylander manufacturer data
    data[7] = 0x00;
    data[8] = 0x46;
    data[9] = 0x59;
    data[10] = 0x25;
    data[11] = 0x58;
    data[12] = 0x49;
    data[13] = 0x10;
    data[14] = 0x23;
    data[15] = 0x02;

    // Block 1: Character ID and variant
    data[0x10] = charId & 0xFF;
    data[0x11] = (charId >> 8) & 0xFF;
    // Bytes 0x12-0x1B reserved
    data[0x1C] = variantId & 0xFF;
    data[0x1D] = (variantId >> 8) & 0xFF;

    // Block 2: Reserved, will contain Type 0 checksum
    // Calculate Type 0 checksum
    uint16_t crc0 = xk::skylanderCrypto::crc16(data, 0x1E);
    data[0x1E] = crc0 & 0xFF;
    data[0x1F] = (crc0 >> 8) & 0xFF;

    // Block 3: Sector 0 trailer (Key A, access bits, Key B)
    const uint8_t SKYKEY[] = {0x4B, 0x0B, 0x20, 0x10, 0x7C, 0xCB};
    memcpy(data + 0x30, SKYKEY, 6);
    data[0x36] = 0xFF;
    data[0x37] = 0x07;
    data[0x38] = 0x80;
    data[0x39] = 0x69;
    // Key B (bytes 0x3A-0x3F) - zeros or same as Key A
    memcpy(data + 0x3A, SKYKEY, 6);
}

void initSectorTrailers(uint8_t* data, const uint8_t* uid)
{
    // Make a non-const copy of UID for makeSkyKey
    uint8_t uidCopy[4];
    memcpy(uidCopy, uid, 4);

    // Set sector trailers for sectors 1-15
    for (int sector = 1; sector < 16; sector++)
    {
        int trailerOffset = sector * 0x40 + 0x30;  // Each sector is 64 bytes, trailer at +48

        // Generate sector key
        uint64_t sectorKey = xk::skylanderNFC::makeSkyKey(sector, uidCopy);
        memcpy(data + trailerOffset, &sectorKey, 6);

        // Access bits
        data[trailerOffset + 6] = 0xFF;
        data[trailerOffset + 7] = 0x07;
        data[trailerOffset + 8] = 0x80;
        data[trailerOffset + 9] = 0x69;

        // Key B (same as Key A)
        memcpy(data + trailerOffset + 10, &sectorKey, 6);
    }
}

void initDataArea(uint8_t* data, uint8_t areaBlock, const SkylanderConfig& config)
{
    uint8_t* header = data + areaBlock * 16;

    // XP (3 bytes, little-endian)
    header[0] = config.xp & 0xFF;
    header[1] = (config.xp >> 8) & 0xFF;
    header[2] = (config.xp >> 16) & 0xFF;

    // Money (2 bytes, little-endian)
    header[3] = config.money & 0xFF;
    header[4] = (config.money >> 8) & 0xFF;

    // Cumulative time (4 bytes) - leave as 0
    // header[5] through header[8] = 0

    // Sequence counter
    header[9] = 0x00;

    // Checksums will be set after all data is written

    // Block +1: Skills, platform, hat
    uint8_t* skillsBlock = data + (areaBlock + 1) * 16;
    skillsBlock[0] = config.skills & 0xFF;
    skillsBlock[1] = (config.skills >> 8) & 0xFF;
    skillsBlock[2] = config.platform;
    skillsBlock[3] = config.hat & 0xFF;
    skillsBlock[4] = (config.hat >> 8) & 0xFF;

    // Blocks +2 and +4: Name (Unicode, 16 bytes each = 8 chars each)
    // Skip block +3 which is sector trailer for area 0
    if (!config.customName.empty())
    {
        // Block +2: First 8 characters
        uint8_t* nameBlock1 = data + (areaBlock + 2) * 16;
        size_t len1 = std::min(config.customName.length(), (size_t)8);
        for (size_t i = 0; i < len1; i++)
        {
            nameBlock1[i * 2] = config.customName[i];
            nameBlock1[i * 2 + 1] = 0;  // ASCII to Unicode
        }

        // Block +4: Next 8 characters (skipping sector trailer at +3)
        if (config.customName.length() > 8)
        {
            uint8_t* nameBlock2 = data + (areaBlock + 4) * 16;
            size_t len2 = std::min(config.customName.length() - 8, (size_t)8);
            for (size_t i = 0; i < len2; i++)
            {
                nameBlock2[i * 2] = config.customName[8 + i];
                nameBlock2[i * 2 + 1] = 0;
            }
        }
    }

    // Block +5: Hero data
    uint8_t* heroBlock = data + (areaBlock + 5) * 16;
    // Heroic challenges bitmask (4 bytes at offset 0)
    heroBlock[0] = config.heroicChallenges & 0xFF;
    heroBlock[1] = (config.heroicChallenges >> 8) & 0xFF;
    heroBlock[2] = (config.heroicChallenges >> 16) & 0xFF;
    heroBlock[3] = (config.heroicChallenges >> 24) & 0xFF;
    // Hero points at offset 6
    heroBlock[6] = config.heroPoints;
}

bool buildSkylander(const SkylanderConfig& config, uint8_t* data)
{
    // Generate or use provided UID
    uint8_t uid[4];
    if (config.hasUID)
    {
        memcpy(uid, config.uid, 4);
    }
    else
    {
        generateRandomUID(uid);
    }

    std::cout << "Building Skylander with UID: "
              << xk::skylanderNFC::toHexStr(uid, 4) << std::endl;

    // Initialize Sector 0 (UID, character ID, variant ID)
    initSector0(data, uid, config.characterId, config.variantId);

    // Initialize sector trailers
    initSectorTrailers(data, uid);

    // Initialize Data Area 0 (block 0x08)
    initDataArea(data, 0x08, config);

    // Initialize Data Area 1 (block 0x24) - mirror of Area 0
    initDataArea(data, 0x24, config);

    // Calculate checksums for both areas (before encryption)
    xk::skylanderCrypto::updateAreaChecksums(data, 0x08);
    xk::skylanderCrypto::updateAreaChecksums(data, 0x24);

    // Encrypt the data blocks
    xk::skylanderCrypto::encryptSkylander(data);

    return true;
}

bool writeToFile(const uint8_t* data, const std::string& filename)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open())
        return false;

    file.write(reinterpret_cast<const char*>(data), 1024);
    file.close();
    return true;
}

bool writeToNFC(const uint8_t* data)
{
    try
    {
        xk::acr122u nfc;

        std::cout << "Looking for ACR122U NFC Reader/Writer..." << std::endl;
        std::string readerName = nfc.findAndConnect();
        std::cout << "  Found: " << readerName << std::endl;
        std::cout << "  Connected." << std::endl;

        // Try to authenticate with default key first (blank card)
        const uint8_t DEFAULTKEY[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        xk::acr122u::NFC_Sector sector0;
        memcpy(sector0.key_A, DEFAULTKEY, 6);

        uint32_t retval = nfc.sectorAuthKeyA(0, sector0.key_A);
        if (retval != NFC_OK)
        {
            std::cout << "Card authentication failed. Is this a blank card?" << std::endl;
            std::cout << "Note: Cannot overwrite existing Skylanders for safety." << std::endl;
            return false;
        }

        // Read current sector 0 to get card UID
        retval = nfc.sectorRead(0, sector0);
        if (retval != NFC_OK)
        {
            std::cout << "Failed to read card." << std::endl;
            return false;
        }

        uint8_t cardUID[4];
        memcpy(cardUID, sector0.blk0, 4);
        std::cout << "Card UID: " << xk::skylanderNFC::toHexStr(cardUID, 4) << std::endl;

        // Rebuild Skylander data with card's actual UID
        uint8_t newData[1024];
        memcpy(newData, data, 1024);

        // Update UID in the data
        memcpy(newData, cardUID, 4);
        newData[4] = cardUID[0] ^ cardUID[1] ^ cardUID[2] ^ cardUID[3];  // BCC

        // Recalculate sector trailers with correct UID
        initSectorTrailers(newData, cardUID);

        // Decrypt, update checksums, re-encrypt
        xk::skylanderCrypto::decryptSkylander(newData);
        xk::skylanderCrypto::updateAreaChecksums(newData, 0x08);
        xk::skylanderCrypto::updateAreaChecksums(newData, 0x24);
        xk::skylanderCrypto::encryptSkylander(newData);

        // Write all sectors
        std::cout << "Writing sectors..." << std::endl;
        xk::acr122u::NFC_Sector writeSector;

        for (int sec = 0; sec < 16; sec++)
        {
            // Copy sector data
            memcpy(&writeSector, newData + sec * 64, 64);

            // Authenticate
            if (sec == 0)
            {
                memcpy(writeSector.key_A, DEFAULTKEY, 6);
            }
            else
            {
                uint64_t sectorKey = xk::skylanderNFC::makeSkyKey(sec, cardUID);
                memcpy(writeSector.key_A, &sectorKey, 6);
            }

            // For sector 0, only write blocks 1-2 (block 0 is read-only UID, block 3 is trailer)
            // For other sectors, write blocks 0-2 (block 3 is trailer)
            if (sec == 0)
            {
                // Authenticate with default key
                retval = nfc.sectorAuthKeyA(0, (uint8_t*)DEFAULTKEY);
                if (retval != NFC_OK)
                {
                    std::cout << "Auth failed for sector 0" << std::endl;
                    return false;
                }

                // Write blocks 1 and 2 only
                retval = nfc.sectorWrite(0, writeSector, 1, 2);
                if (retval != NFC_OK)
                {
                    std::cout << "Write failed for sector 0" << std::endl;
                    return false;
                }

                // Write sector trailer to convert to Skylander keys
                retval = nfc.sectorWrite(0, writeSector, 3, 1);
                if (retval != NFC_OK)
                {
                    std::cout << "Failed to write sector 0 trailer" << std::endl;
                    return false;
                }
            }
            else
            {
                // For other sectors, use default key first (blank card)
                retval = nfc.sectorAuthKeyA(sec, (uint8_t*)DEFAULTKEY);
                if (retval != NFC_OK)
                {
                    std::cout << "Auth failed for sector " << sec << std::endl;
                    return false;
                }

                // Write all 4 blocks
                retval = nfc.sectorWrite(sec, writeSector);
                if (retval != NFC_OK)
                {
                    std::cout << "Write failed for sector " << sec << std::endl;
                    return false;
                }
            }

            std::cout << "." << std::flush;
        }
        std::cout << std::endl;

        nfc.deviceDisconnect();
        return true;
    }
    catch (xk::appError* e)
    {
        std::cout << "Error: " << e->what() << std::endl;
        delete e;
        return false;
    }
}
