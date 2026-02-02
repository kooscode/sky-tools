/*
 * sky-identify - Identify Skylanders from NFC reader or dump files
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <vector>

#include "skylanderDB.hpp"
#include "skylanderCrypto.hpp"
#include "hatDB.hpp"
#include "acr122u.hpp"
#include "skylanderNFC.hpp"
#include "appError.hpp"

namespace fs = std::filesystem;

bool g_verbose = false;

// Convert bytes to hex string
std::string toHexStr(const uint8_t* data, size_t len)
{
    std::stringstream ss;
    for (size_t i = 0; i < len; i++)
    {
        ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)data[i];
    }
    return ss.str();
}

// List all Skylanders in the database (sorted by game, type, element, name)
void listSkylanders(const std::string& filter = "")
{
    const xk::SkylanderInfo* db = xk::skylanderDB::getDatabase();
    size_t count = xk::skylanderDB::getDatabaseSize();

    std::string lowerFilter = filter;
    std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(),
        [](unsigned char c) { return std::tolower(c); });

    // Collect entries into a vector for sorting
    std::vector<const xk::SkylanderInfo*> entries;
    for (size_t i = 0; i < count; i++)
    {
        if (db[i].name == nullptr) continue;

        // Apply filter if specified
        if (!lowerFilter.empty())
        {
            std::string name = db[i].name;
            std::transform(name.begin(), name.end(), name.begin(),
                [](unsigned char c) { return std::tolower(c); });
            std::string type = xk::skylanderDB::getTypeString(db[i].type);
            std::transform(type.begin(), type.end(), type.begin(),
                [](unsigned char c) { return std::tolower(c); });
            std::string element = xk::skylanderDB::getElementString(db[i].element);
            std::transform(element.begin(), element.end(), element.begin(),
                [](unsigned char c) { return std::tolower(c); });
            std::string game = xk::skylanderDB::getGameString(db[i].game);
            std::transform(game.begin(), game.end(), game.begin(),
                [](unsigned char c) { return std::tolower(c); });

            if (name.find(lowerFilter) == std::string::npos &&
                type.find(lowerFilter) == std::string::npos &&
                element.find(lowerFilter) == std::string::npos &&
                game.find(lowerFilter) == std::string::npos)
            {
                continue;
            }
        }

        entries.push_back(&db[i]);
    }

    // Sort by game, type, element, name
    std::sort(entries.begin(), entries.end(),
        [](const xk::SkylanderInfo* a, const xk::SkylanderInfo* b) {
            if (a->game != b->game) return a->game < b->game;
            if (a->type != b->type) return a->type < b->type;
            if (a->element != b->element) return a->element < b->element;
            return strcmp(a->name, b->name) < 0;
        });

    std::cout << "=== Skylander Database ===" << std::endl;
    std::cout << std::left << std::setw(18) << "Game"
              << std::setw(18) << "Type"
              << std::setw(10) << "Element"
              << std::setw(30) << "Skylander Name"
              << "ID" << std::endl;
    std::cout << std::string(90, '-') << std::endl;

    for (const auto* entry : entries)
    {
        std::cout << std::left
                  << std::setw(18) << xk::skylanderDB::getGameString(entry->game)
                  << std::setw(18) << xk::skylanderDB::getTypeString(entry->type)
                  << std::setw(10) << xk::skylanderDB::getElementString(entry->element)
                  << std::setw(30) << entry->name
                  << entry->id << std::endl;
    }

    std::cout << std::string(90, '-') << std::endl;
    std::cout << "Total: " << entries.size() << " Skylanders";
    if (!filter.empty())
        std::cout << " (filtered from " << count << ")";
    std::cout << std::endl;
}

// List all hats in the database
void listHats(const std::string& filter = "")
{
    const xk::HatInfo* hats = xk::hatDB::getHatList();
    size_t count = xk::hatDB::getHatCount();

    std::string lowerFilter = filter;
    std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(),
        [](unsigned char c) { return std::tolower(c); });

    std::cout << "=== Hat Database ===" << std::endl;
    std::cout << std::left << std::setw(6) << "ID" << "Name" << std::endl;
    std::cout << std::string(50, '-') << std::endl;

    int displayed = 0;
    for (size_t i = 0; i < count; i++)
    {
        if (hats[i].name == nullptr) continue;

        // Apply filter if specified
        if (!lowerFilter.empty())
        {
            std::string name = hats[i].name;
            std::transform(name.begin(), name.end(), name.begin(),
                [](unsigned char c) { return std::tolower(c); });
            if (name.find(lowerFilter) == std::string::npos)
                continue;
        }

        std::cout << std::left << std::setw(6) << hats[i].id << hats[i].name << std::endl;
        displayed++;
    }

    std::cout << std::string(50, '-') << std::endl;
    std::cout << "Total: " << displayed << " hats";
    if (!filter.empty())
        std::cout << " (filtered from " << count << ")";
    std::cout << std::endl;
}

// Data area field offsets (relative to area start block)
#define DATA_XP_OFFSET      0x00
#define DATA_MONEY_OFFSET   0x03
#define DATA_PLAYTIME_OFFSET 0x05
#define DATA_SEQUENCE_OFFSET 0x09

// Block 1 offsets (skills/hat block = area + 0x10)
#define DATA_SKILLS_OFFSET  0x10
#define DATA_PLATFORM_OFFSET 0x12
#define DATA_HAT_OFFSET     0x13

// Name offsets (area + 0x20 for part 1, area + 0x40 for part 2)
#define DATA_NAME1_OFFSET   0x20
#define DATA_NAME2_OFFSET   0x40

// Hero data (area + 0x50)
#define DATA_HERO_CHALLENGES_OFFSET 0x50
#define DATA_HERO_POINTS_OFFSET 0x56

// Extract 24-bit little-endian value
uint32_t read24LE(const uint8_t* data)
{
    return data[0] | (data[1] << 8) | (data[2] << 16);
}

// Extract 16-bit little-endian value
uint16_t read16LE(const uint8_t* data)
{
    return data[0] | (data[1] << 8);
}

// Extract 32-bit little-endian value
uint32_t read32LE(const uint8_t* data)
{
    return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}

// Convert UTF-16LE to ASCII string
std::string utf16ToAscii(const uint8_t* data, size_t maxChars)
{
    std::string result;
    for (size_t i = 0; i < maxChars; i++)
    {
        uint16_t ch = data[i * 2] | (data[i * 2 + 1] << 8);
        if (ch == 0) break;
        if (ch < 128)
            result += static_cast<char>(ch);
        else
            result += '?'; // Non-ASCII
    }
    return result;
}

// Format playtime as hours:minutes:seconds
std::string formatPlaytime(uint32_t seconds)
{
    uint32_t hours = seconds / 3600;
    uint32_t minutes = (seconds % 3600) / 60;
    uint32_t secs = seconds % 60;

    std::stringstream ss;
    ss << hours << "h " << minutes << "m " << secs << "s";
    return ss.str();
}

// Get platform name from ID
const char* getPlatformName(uint8_t platformId)
{
    switch (platformId)
    {
        case 0x00: return "None";
        case 0x01: return "Wii";
        case 0x02: return "Xbox 360";
        case 0x03: return "PlayStation 3";
        case 0x04: return "PC";
        case 0x05: return "3DS";
        case 0x06: return "Wii U";
        case 0x07: return "Xbox One";
        case 0x08: return "PlayStation 4";
        default:   return "Unknown";
    }
}

// Display full Skylander info from raw data (decrypted)
void displayInfo(const uint8_t* rawData, const std::string& source, bool fullDump)
{
    // Make a copy for decryption
    uint8_t data[1024];
    memcpy(data, rawData, 1024);

    // Get character info from sector 0 (not encrypted)
    uint16_t charId = xk::skylanderDB::getCharacterId(data);
    uint16_t variantId = xk::skylanderDB::getVariantId(data);
    const xk::SkylanderInfo* info = xk::skylanderDB::getInfo(charId);

    // Extract UID (first 4 bytes)
    std::string uid = toHexStr(data, 4);

    // Decrypt the data to read stats
    xk::skylanderCrypto::decryptSkylander(data);

    // Determine active data area
    uint8_t activeArea = xk::skylanderCrypto::getActiveArea(data);
    size_t areaOffset = activeArea * 16; // Convert block to byte offset

    // Read data from active area
    uint32_t xp = read24LE(data + areaOffset + DATA_XP_OFFSET);
    uint16_t money = read16LE(data + areaOffset + DATA_MONEY_OFFSET);
    uint32_t playtime = read32LE(data + areaOffset + DATA_PLAYTIME_OFFSET);
    uint8_t sequence = data[areaOffset + DATA_SEQUENCE_OFFSET];

    // Skills/Hat block
    uint16_t skills = read16LE(data + areaOffset + DATA_SKILLS_OFFSET);
    uint8_t platform = data[areaOffset + DATA_PLATFORM_OFFSET];
    uint16_t hatId = read16LE(data + areaOffset + DATA_HAT_OFFSET);

    // Custom name (16 chars max, UTF-16LE) - only for regular playable figures
    std::string customName;
    if (!xk::skylanderDB::isTrap(charId) &&
        !xk::skylanderDB::isCreationCrystal(charId) &&
        !xk::skylanderDB::isVehicle(charId) &&
        !(info && info->type == xk::SKY_TYPE_ITEM))
    {
        customName = utf16ToAscii(data + areaOffset + DATA_NAME1_OFFSET, 8);
        customName += utf16ToAscii(data + areaOffset + DATA_NAME2_OFFSET, 8);
    }

    // Hero data
    uint32_t heroChallenges = read32LE(data + areaOffset + DATA_HERO_CHALLENGES_OFFSET);
    uint8_t heroPoints = data[areaOffset + DATA_HERO_POINTS_OFFSET];

    // Get hat name
    const char* hatName = xk::hatDB::getHatName(hatId);

    // Check if this is a special type (trap, crystal, vehicle)
    bool isTrap = xk::skylanderDB::isTrap(charId);
    bool isCrystal = xk::skylanderDB::isCreationCrystal(charId);
    bool isVehicle = xk::skylanderDB::isVehicle(charId);
    bool isItem = (info && info->type == xk::SKY_TYPE_ITEM);

    if (g_verbose || fullDump)
    {
        // Detailed output
        std::cout << "=== Skylander Information ===" << std::endl;
        if (!source.empty())
            std::cout << "Source:          " << source << std::endl;
        std::cout << std::endl;

        std::cout << "-- Identity --" << std::endl;
        std::cout << "UID:             " << uid << std::endl;
        std::cout << "Character ID:    " << charId << " (0x" << std::hex << charId << std::dec << ")" << std::endl;
        std::cout << "Variant ID:      " << variantId << " (0x" << std::hex << variantId << std::dec << ")" << std::endl;

        if (info)
        {
            std::cout << "Name:            " << info->name << std::endl;
            std::cout << "Type:            " << xk::skylanderDB::getTypeString(info->type) << std::endl;
            std::cout << "Element:         " << xk::skylanderDB::getElementString(info->element) << std::endl;
            std::cout << "Game:            " << xk::skylanderDB::getGameString(info->game) << std::endl;
        }
        else
        {
            std::cout << "Name:            UNKNOWN" << std::endl;
        }

        // Stats only for playable figures (not traps, crystals, items, vehicles)
        if (!isTrap && !isCrystal && !isItem && !isVehicle)
        {
            std::cout << std::endl;
            std::cout << "-- Stats --" << std::endl;
            std::cout << "XP:              " << xp << std::endl;
            std::cout << "Money:           " << money << std::endl;
            std::cout << "Hero Points:     " << (int)heroPoints << std::endl;
            std::cout << "Playtime:        " << formatPlaytime(playtime) << " (" << playtime << "s)" << std::endl;

            std::cout << std::endl;
            std::cout << "-- Equipment --" << std::endl;
            std::cout << "Hat:             ";
            if (hatId == 0)
                std::cout << "None" << std::endl;
            else
                std::cout << hatName << " (ID: " << hatId << ")" << std::endl;

            if (!customName.empty())
                std::cout << "Custom Name:     " << customName << std::endl;

            std::cout << std::endl;
            std::cout << "-- Progress --" << std::endl;
            std::cout << "Skill Upgrades:  0x" << std::hex << std::setw(4) << std::setfill('0') << skills << std::dec << std::endl;
            std::cout << "Hero Challenges: 0x" << std::hex << std::setw(8) << std::setfill('0') << heroChallenges << std::dec << std::endl;
            std::cout << "Last Platform:   " << getPlatformName(platform) << std::endl;
        }
        else if (isTrap)
        {
            std::cout << std::endl;
            std::cout << "-- Trap Data --" << std::endl;
            std::cout << "Note: Villain data is stored in variant ID" << std::endl;
        }
        else if (isCrystal)
        {
            std::cout << std::endl;
            std::cout << "-- Creation Crystal --" << std::endl;
            std::cout << "Note: Imaginator data is stored in encrypted format" << std::endl;
        }

        std::cout << std::endl;
        std::cout << "-- Internal --" << std::endl;
        std::cout << "Active Area:     " << (activeArea == 0x08 ? "0 (block 0x08)" : "1 (block 0x24)") << std::endl;
        std::cout << "Sequence:        " << (int)sequence << std::endl;

        std::cout << std::endl;
    }
    else
    {
        // Compact output
        if (info)
        {
            std::cout << info->name;
            if (!customName.empty())
                std::cout << " \"" << customName << "\"";
            std::cout << " | " << xk::skylanderDB::getTypeString(info->type)
                      << " | " << xk::skylanderDB::getElementString(info->element)
                      << " | " << xk::skylanderDB::getGameString(info->game);

            if (!isTrap && !isCrystal && !isItem && !isVehicle)
            {
                std::cout << " | XP:" << xp
                          << " | $:" << money
                          << " | HP:" << (int)heroPoints;
                if (hatId > 0)
                    std::cout << " | Hat:" << hatName;
            }

            std::cout << " | UID:" << uid;
            if (!source.empty())
                std::cout << " | " << source;
            std::cout << std::endl;
        }
        else
        {
            std::cout << "UNKNOWN | ID:" << charId << " | Var:" << variantId << " | UID:" << uid;
            if (!source.empty())
                std::cout << " | " << source;
            std::cout << std::endl;
        }
    }
}

void identifyFile(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "ERROR: " << filepath << " - Could not open file" << std::endl;
        return;
    }

    // Check file size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize != 1024)
    {
        std::cout << "ERROR: " << filepath << " - Invalid size (" << fileSize << " bytes)" << std::endl;
        return;
    }

    // Read file
    uint8_t data[1024];
    file.read(reinterpret_cast<char*>(data), 1024);
    file.close();

    // Display info
    std::string filename = fs::path(filepath).filename().string();
    displayInfo(data, g_verbose ? filepath : filename, false);
}

bool identifyFromReader()
{
    const uint8_t SKYKEY[] = SKYLANDER_BLOCK_0_KEY_A;
    const uint8_t DEFAULTKEY[] = NFC_DEFAULT_KEY_A;
    uint8_t data[1024] = {0};
    xk::acr122u::NFC_Sector sector;

    try
    {
        xk::acr122u nfc;

        std::cout << "Looking for ACR122U NFC Reader..." << std::endl;
        std::string reader_name = nfc.findAndConnect();
        std::cout << "Found: " << reader_name << std::endl;
        std::cout << "Place Skylander on reader..." << std::endl;

        // Read sector 0 first (uses fixed Skylander key)
        memcpy(sector.key_A, SKYKEY, NFC_KEY_SIZE);
        uint32_t retval = nfc.sectorAuthKeyA(0, sector.key_A);

        // If Skylander key fails, try default key
        if (retval == NFC_ERROR_KEYA_AUTH)
        {
            memcpy(sector.key_A, DEFAULTKEY, NFC_KEY_SIZE);
            retval = nfc.sectorAuthKeyA(0, sector.key_A);
        }

        if (retval != NFC_OK)
        {
            std::cout << "ERROR: Authentication failed - not a valid Skylander" << std::endl;
            return false;
        }

        // Read sector 0
        retval = nfc.sectorRead(0, sector);
        if (retval != NFC_OK)
        {
            std::cout << "ERROR: Could not read sector 0" << std::endl;
            return false;
        }
        memcpy(data, &sector, sizeof(sector));

        std::cout << "Reading full card..." << std::endl;

        // Read remaining sectors (1-15) using generated keys
        for (int s = 1; s < 16; s++)
        {
            // Generate sector key from UID (first 4 bytes)
            uint64_t sectorKey = xk::skylanderNFC::makeSkyKey(s, data);
            memcpy(sector.key_A, &sectorKey, NFC_KEY_SIZE);

            retval = nfc.sectorAuthKeyA(s, sector.key_A);
            if (retval != NFC_OK)
            {
                std::cout << "Warning: Could not auth sector " << s << std::endl;
                continue;
            }

            retval = nfc.sectorRead(s, sector);
            if (retval != NFC_OK)
            {
                std::cout << "Warning: Could not read sector " << s << std::endl;
                continue;
            }

            // Copy sector data (4 blocks * 16 bytes = 64 bytes per sector)
            memcpy(data + s * 64, &sector, sizeof(sector));
        }

        std::cout << std::endl;

        // Display full info
        displayInfo(data, "NFC Reader", true);

        nfc.deviceDisconnect();
        return true;
    }
    catch (xk::appError* e)
    {
        std::cout << "ERROR: " << e->what() << std::endl;
        delete e;
        return false;
    }
}

int main(int argc, char** argv)
{
    bool hasFiles = false;
    bool doListSkylanders = false;
    bool doListHats = false;
    std::string listFilter;

    // Check for flags and count file arguments
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "-v" || arg == "--verbose")
        {
            g_verbose = true;
        }
        else if (arg == "-l" || arg == "--list-skylanders")
        {
            doListSkylanders = true;
        }
        else if (arg == "-L" || arg == "--list-hats")
        {
            doListHats = true;
        }
        else if (arg == "-f" || arg == "--filter")
        {
            if (i + 1 < argc)
            {
                listFilter = argv[++i];
            }
        }
        else if (arg == "-h" || arg == "--help")
        {
            std::cout << "Usage: sky-identify [options] [file_or_directory ...]" << std::endl;
            std::cout << "Identifies Skylanders from NFC reader or dump files" << std::endl;
            std::cout << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -v, --verbose          Verbose output (detailed info)" << std::endl;
            std::cout << "  -l, --list-skylanders  List all Skylanders in database" << std::endl;
            std::cout << "  -L, --list-hats        List all hats in database" << std::endl;
            std::cout << "  -f, --filter TERM      Filter list by name/type/element/game" << std::endl;
            std::cout << "  -h, --help             Show this help" << std::endl;
            std::cout << std::endl;
            std::cout << "If no files specified, reads from NFC reader." << std::endl;
            std::cout << "Supports .bin, .dump, .dmp files and directories." << std::endl;
            std::cout << std::endl;
            std::cout << "Examples:" << std::endl;
            std::cout << "  sky-identify                    Read from NFC reader" << std::endl;
            std::cout << "  sky-identify -l                 List all Skylanders" << std::endl;
            std::cout << "  sky-identify -l -f fire         List fire element Skylanders" << std::endl;
            std::cout << "  sky-identify -L                 List all hats" << std::endl;
            std::cout << "  sky-identify dump.bin           Identify from dump file" << std::endl;
            std::cout << "  sky-identify ./dumps/           Identify all dumps in directory" << std::endl;
            return 0;
        }
        else
        {
            hasFiles = true;
        }
    }

    // Handle list options
    if (doListSkylanders)
    {
        listSkylanders(listFilter);
        return 0;
    }

    if (doListHats)
    {
        listHats(listFilter);
        return 0;
    }

    // If no file arguments, read from NFC reader
    if (!hasFiles)
    {
        return identifyFromReader() ? 0 : -1;
    }

    // Process file/directory arguments
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "-v" || arg == "--verbose" ||
            arg == "-l" || arg == "--list-skylanders" ||
            arg == "-L" || arg == "--list-hats")
        {
            continue;
        }

        if (arg == "-f" || arg == "--filter")
        {
            i++; // Skip filter value
            continue;
        }

        if (fs::is_directory(arg))
        {
            // Recursively process directory
            for (const auto& entry : fs::recursive_directory_iterator(arg))
            {
                if (entry.is_regular_file())
                {
                    std::string ext = entry.path().extension().string();
                    if (ext == ".bin" || ext == ".dump" || ext == ".dmp")
                    {
                        identifyFile(entry.path().string());
                    }
                }
            }
        }
        else if (fs::is_regular_file(arg))
        {
            identifyFile(arg);
        }
        else
        {
            std::cout << "ERROR: " << arg << " - Not found" << std::endl;
        }
    }

    return 0;
}
