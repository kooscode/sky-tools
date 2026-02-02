/*
 * Skylander Character Database
 *
 * Maps character IDs to names for identification.
 * Character ID is stored at offset 0x10 (2 bytes, little-endian)
 * Variant ID is stored at offset 0x1C (2 bytes, bitfield)
 *
 * Covers all Skylander games:
 * - Spyro's Adventure (2011)
 * - Giants (2012)
 * - Swap Force (2013)
 * - Trap Team (2014)
 * - SuperChargers (2015)
 * - Imaginators (2016)
 */

#ifndef SKYLANDER_DB_HPP
#define SKYLANDER_DB_HPP

#include <stdint.h>
#include <string>
#include <map>

namespace xk
{
    // Character ID offset in block 1 (offset 0x10 from start of card)
    #define SKY_CHARACTER_ID_OFFSET 0x10
    #define SKY_VARIANT_ID_OFFSET   0x1C

    // Skylander types
    enum SkylanderType
    {
        SKY_TYPE_UNKNOWN = 0,
        SKY_TYPE_SKYLANDER,      // Regular Skylander figure
        SKY_TYPE_GIANT,          // Giants (larger figures)
        SKY_TYPE_SWAP_TOP,       // Swap Force top half
        SKY_TYPE_SWAP_BOTTOM,    // Swap Force bottom half
        SKY_TYPE_TRAP_MASTER,    // Trap Team trap masters
        SKY_TYPE_MINI,           // Mini figures
        SKY_TYPE_SUPERCHARGER,   // SuperChargers drivers
        SKY_TYPE_VEHICLE,        // SuperChargers vehicles
        SKY_TYPE_TRAP,           // Trap Team traps
        SKY_TYPE_ITEM,           // Magic items / Adventure packs
        SKY_TYPE_IMAGINATOR,     // Imaginators senseis
        SKY_TYPE_CREATION_CRYSTAL // Imaginators creation crystals
    };

    // Game generations
    enum SkylanderGame
    {
        SKY_GAME_UNKNOWN = 0,
        SKY_GAME_SPYROS_ADVENTURE,  // 2011
        SKY_GAME_GIANTS,            // 2012
        SKY_GAME_SWAP_FORCE,        // 2013
        SKY_GAME_TRAP_TEAM,         // 2014
        SKY_GAME_SUPERCHARGERS,     // 2015
        SKY_GAME_IMAGINATORS        // 2016
    };

    // Elements
    enum SkylanderElement
    {
        SKY_ELEMENT_NONE = 0,
        SKY_ELEMENT_AIR,
        SKY_ELEMENT_EARTH,
        SKY_ELEMENT_FIRE,
        SKY_ELEMENT_WATER,
        SKY_ELEMENT_MAGIC,
        SKY_ELEMENT_TECH,
        SKY_ELEMENT_LIFE,
        SKY_ELEMENT_UNDEAD,
        SKY_ELEMENT_LIGHT,
        SKY_ELEMENT_DARK,
        SKY_ELEMENT_KAOS
    };

    struct SkylanderInfo
    {
        uint16_t id;
        const char* name;
        SkylanderType type;
        SkylanderElement element;
        SkylanderGame game;
    };

    class skylanderDB
    {
    public:
        skylanderDB() = delete;

        // Look up Skylander name by character ID
        static std::string getName(uint16_t characterId);

        // Look up full Skylander info by character ID
        static const SkylanderInfo* getInfo(uint16_t characterId);

        // Get Skylander type as string
        static const char* getTypeString(SkylanderType type);

        // Get game/generation as string
        static const char* getGameString(SkylanderGame game);

        // Get element as string
        static const char* getElementString(SkylanderElement element);

        // Get character ID from raw card data
        static uint16_t getCharacterId(const uint8_t* data);

        // Get variant ID from raw card data
        static uint16_t getVariantId(const uint8_t* data);

        // Check if this is a Creation Crystal
        static bool isCreationCrystal(uint16_t characterId);

        // Check if this is a Trap
        static bool isTrap(uint16_t characterId);

        // Check if this is a Vehicle
        static bool isVehicle(uint16_t characterId);

        // Get the full database array
        static const SkylanderInfo* getDatabase();

        // Get the number of entries in the database
        static size_t getDatabaseSize();

        // Find Skylander by name (case-insensitive)
        static const SkylanderInfo* findByName(const std::string& name);
    };
}

#endif /* SKYLANDER_DB_HPP */
