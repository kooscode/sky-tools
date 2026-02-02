/*
 * Skylander Hat Database
 * Contains all hats from Skylanders games (Spyro's Adventure through Imaginators)
 */

#ifndef HATDB_HPP
#define HATDB_HPP

#include <stdint.h>
#include <string>

namespace xk
{
    struct HatInfo
    {
        uint16_t id;
        const char* name;
    };

    class hatDB
    {
    public:
        hatDB() = delete;

        // Get the full hat list
        static const HatInfo* getHatList();

        // Get number of hats in database
        static size_t getHatCount();

        // Get hat info by ID (returns nullptr if not found)
        static const HatInfo* getHatById(uint16_t hatId);

        // Get hat name by ID (returns "Unknown" if not found)
        static const char* getHatName(uint16_t hatId);

        // Find hat by name (case-insensitive, returns nullptr if not found)
        static const HatInfo* findHatByName(const std::string& name);
    };
}

#endif /* HATDB_HPP */
