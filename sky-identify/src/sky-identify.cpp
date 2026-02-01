/*
 * sky-identify - Identify Skylanders from dump files
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

#include "skylanderDB.hpp"

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

    // Get character info
    uint16_t charId = xk::skylanderDB::getCharacterId(data);
    uint16_t variantId = xk::skylanderDB::getVariantId(data);
    const xk::SkylanderInfo* info = xk::skylanderDB::getInfo(charId);

    // Extract UID (first 4 bytes)
    std::string uid = toHexStr(data, 4);

    // Extract filename from path
    std::string filename = fs::path(filepath).filename().string();

    if (g_verbose)
    {
        // Verbose output - one file at a time with full details
        std::cout << "File: " << filepath << std::endl;
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
        // Compact output - pipe-delimited for easy parsing
        if (info)
        {
            std::cout << info->name << " | "
                      << xk::skylanderDB::getTypeString(info->type) << " | "
                      << xk::skylanderDB::getElementString(info->element) << " | "
                      << xk::skylanderDB::getGameString(info->game) << " | "
                      << "ID:" << charId << " | "
                      << "Var:" << variantId << " | "
                      << "UID:" << uid << " | "
                      << filename << std::endl;
        }
        else
        {
            std::cout << "UNKNOWN | ID:" << charId << " | Var:" << variantId << " | UID:" << uid << " | " << filename << std::endl;
        }
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: sky-identify [-v] <file_or_directory> [...]" << std::endl;
        std::cout << "Identifies Skylanders from dump files (.bin, .dump, .dmp)" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -v    Verbose output (detailed info per file)" << std::endl;
        return -1;
    }

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "-v" || arg == "--verbose")
        {
            g_verbose = true;
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
