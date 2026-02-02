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

#include "skylanderDB.hpp"
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

// Display Skylander info from raw data
void displayInfo(const uint8_t* data, const std::string& source)
{
    // Get character info
    uint16_t charId = xk::skylanderDB::getCharacterId(data);
    uint16_t variantId = xk::skylanderDB::getVariantId(data);
    const xk::SkylanderInfo* info = xk::skylanderDB::getInfo(charId);

    // Extract UID (first 4 bytes)
    std::string uid = toHexStr(data, 4);

    if (g_verbose)
    {
        // Verbose output
        if (!source.empty())
            std::cout << "Source: " << source << std::endl;
        std::cout << "  UID:     " << uid << std::endl;
        if (info)
        {
            std::cout << "  Name:    " << info->name << std::endl;
            std::cout << "  Type:    " << xk::skylanderDB::getTypeString(info->type) << std::endl;
            std::cout << "  Element: " << xk::skylanderDB::getElementString(info->element) << std::endl;
            std::cout << "  Game:    " << xk::skylanderDB::getGameString(info->game) << std::endl;
            std::cout << "  Char ID: " << charId << " (0x" << std::hex << charId << std::dec << ")" << std::endl;
            std::cout << "  Variant: " << variantId << " (0x" << std::hex << variantId << std::dec << ")" << std::endl;

            if (info->type == xk::SKY_TYPE_TRAP)
            {
                std::cout << "  Note:    Trap - villain data encoded in variant ID" << std::endl;
            }
            else if (info->type == xk::SKY_TYPE_CREATION_CRYSTAL)
            {
                std::cout << "  Note:    Creation Crystal - stores Imaginator character data" << std::endl;
            }
        }
        else
        {
            std::cout << "  Name:    UNKNOWN" << std::endl;
            std::cout << "  Char ID: " << charId << " (0x" << std::hex << charId << std::dec << ")" << std::endl;
            std::cout << "  Variant: " << variantId << " (0x" << std::hex << variantId << std::dec << ")" << std::endl;
        }
        std::cout << std::endl;
    }
    else
    {
        // Compact output
        if (info)
        {
            std::cout << info->name << " | "
                      << xk::skylanderDB::getTypeString(info->type) << " | "
                      << xk::skylanderDB::getElementString(info->element) << " | "
                      << xk::skylanderDB::getGameString(info->game) << " | "
                      << "ID:" << charId << " | "
                      << "Var:" << variantId << " | "
                      << "UID:" << uid;
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
    displayInfo(data, g_verbose ? filepath : filename);
}

bool identifyFromReader()
{
    const uint8_t SKYKEY[] = SKYLANDER_BLOCK_0_KEY_A;
    const uint8_t DEFAULTKEY[] = NFC_DEFAULT_KEY_A;
    xk::acr122u::NFC_Sector sector0;

    try
    {
        xk::acr122u nfc;

        std::cout << "Looking for ACR122U NFC Reader..." << std::endl;
        std::string reader_name = nfc.findAndConnect();
        std::cout << "Found: " << reader_name << std::endl;
        std::cout << "Place Skylander on reader..." << std::endl;

        // Try Skylander key first
        memcpy(sector0.key_A, SKYKEY, NFC_KEY_SIZE);
        uint32_t retval = nfc.sectorAuthKeyA(0, sector0.key_A);

        // If Skylander key fails, try default key
        if (retval == NFC_ERROR_KEYA_AUTH)
        {
            memcpy(sector0.key_A, DEFAULTKEY, NFC_KEY_SIZE);
            retval = nfc.sectorAuthKeyA(0, sector0.key_A);
        }

        if (retval != NFC_OK)
        {
            std::cout << "ERROR: Authentication failed - not a valid Skylander" << std::endl;
            return false;
        }

        // Read sector 0
        retval = nfc.sectorRead(0, sector0);
        if (retval != NFC_OK)
        {
            std::cout << "ERROR: Could not read card data" << std::endl;
            return false;
        }

        std::cout << std::endl;

        // Display info from sector 0 data
        uint8_t data[1024] = {0};
        memcpy(data, &sector0, sizeof(sector0));
        displayInfo(data, "NFC Reader");

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

    // Check for verbose flag and count file arguments
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "-v" || arg == "--verbose")
        {
            g_verbose = true;
        }
        else if (arg == "-h" || arg == "--help")
        {
            std::cout << "Usage: sky-identify [-v] [file_or_directory ...]" << std::endl;
            std::cout << "Identifies Skylanders from NFC reader or dump files" << std::endl;
            std::cout << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -v    Verbose output (detailed info)" << std::endl;
            std::cout << "  -h    Show this help" << std::endl;
            std::cout << std::endl;
            std::cout << "If no files specified, reads from NFC reader." << std::endl;
            std::cout << "Supports .bin, .dump, .dmp files and directories." << std::endl;
            return 0;
        }
        else
        {
            hasFiles = true;
        }
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

        if (arg == "-v" || arg == "--verbose")
        {
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
