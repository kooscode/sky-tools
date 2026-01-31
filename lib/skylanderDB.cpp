/*
 * Skylander Character Database Implementation
 *
 * Data sourced from community reverse engineering efforts:
 * - https://gist.github.com/skylandersNFC/4f0348c7e66fe9ab28e2ac3b82e549e2
 * - https://github.com/Texthead1/Skylander-IDs
 */

#include "skylanderDB.hpp"

namespace xk
{
    // Complete Skylander character database
    // Format: { ID, "Name", Type, "Element" }
    static const SkylanderInfo SKYLANDER_DATABASE[] =
    {
        // ==================== Spyro's Adventure (2011) ====================
        { 0, "Whirlwind", SKY_TYPE_SKYLANDER, "Air" },
        { 1, "Sonic Boom", SKY_TYPE_SKYLANDER, "Air" },
        { 2, "Warnado", SKY_TYPE_SKYLANDER, "Air" },
        { 3, "Lightning Rod", SKY_TYPE_SKYLANDER, "Air" },
        { 4, "Bash", SKY_TYPE_SKYLANDER, "Earth" },
        { 5, "Terrafin", SKY_TYPE_SKYLANDER, "Earth" },
        { 6, "Dino-Rang", SKY_TYPE_SKYLANDER, "Earth" },
        { 7, "Prism Break", SKY_TYPE_SKYLANDER, "Earth" },
        { 8, "Sunburn", SKY_TYPE_SKYLANDER, "Fire" },
        { 9, "Eruptor", SKY_TYPE_SKYLANDER, "Fire" },
        { 10, "Ignitor", SKY_TYPE_SKYLANDER, "Fire" },
        { 11, "Flameslinger", SKY_TYPE_SKYLANDER, "Fire" },
        { 12, "Zap", SKY_TYPE_SKYLANDER, "Water" },
        { 13, "Wham-Shell", SKY_TYPE_SKYLANDER, "Water" },
        { 14, "Gill Grunt", SKY_TYPE_SKYLANDER, "Water" },
        { 15, "Slam Bam", SKY_TYPE_SKYLANDER, "Water" },
        { 16, "Spyro", SKY_TYPE_SKYLANDER, "Magic" },
        { 17, "Voodood", SKY_TYPE_SKYLANDER, "Magic" },
        { 18, "Double Trouble", SKY_TYPE_SKYLANDER, "Magic" },
        { 19, "Trigger Happy", SKY_TYPE_SKYLANDER, "Tech" },
        { 20, "Drobot", SKY_TYPE_SKYLANDER, "Tech" },
        { 21, "Drill Sergeant", SKY_TYPE_SKYLANDER, "Tech" },
        { 22, "Boomer", SKY_TYPE_SKYLANDER, "Tech" },
        { 23, "Wrecking Ball", SKY_TYPE_SKYLANDER, "Magic" },
        { 24, "Camo", SKY_TYPE_SKYLANDER, "Life" },
        { 25, "Zook", SKY_TYPE_SKYLANDER, "Life" },
        { 26, "Stealth Elf", SKY_TYPE_SKYLANDER, "Life" },
        { 27, "Stump Smash", SKY_TYPE_SKYLANDER, "Life" },
        { 28, "Dark Spyro", SKY_TYPE_SKYLANDER, "Magic" },
        { 29, "Hex", SKY_TYPE_SKYLANDER, "Undead" },
        { 30, "Chop Chop", SKY_TYPE_SKYLANDER, "Undead" },
        { 31, "Ghost Roaster", SKY_TYPE_SKYLANDER, "Undead" },
        { 32, "Cynder", SKY_TYPE_SKYLANDER, "Undead" },

        // ==================== Giants (2012) ====================
        { 100, "Jet-Vac", SKY_TYPE_SKYLANDER, "Air" },
        { 101, "Swarm", SKY_TYPE_GIANT, "Air" },
        { 102, "Crusher", SKY_TYPE_GIANT, "Earth" },
        { 103, "Flashwing", SKY_TYPE_SKYLANDER, "Earth" },
        { 104, "Hot Head", SKY_TYPE_GIANT, "Fire" },
        { 105, "Hot Dog", SKY_TYPE_SKYLANDER, "Fire" },
        { 106, "Chill", SKY_TYPE_SKYLANDER, "Water" },
        { 107, "Thumpback", SKY_TYPE_GIANT, "Water" },
        { 108, "Pop Fizz", SKY_TYPE_SKYLANDER, "Magic" },
        { 109, "Ninjini", SKY_TYPE_GIANT, "Magic" },
        { 110, "Bouncer", SKY_TYPE_GIANT, "Tech" },
        { 111, "Sprocket", SKY_TYPE_SKYLANDER, "Tech" },
        { 112, "Tree Rex", SKY_TYPE_GIANT, "Life" },
        { 113, "Shroomboom", SKY_TYPE_SKYLANDER, "Life" },
        { 114, "Eye-Brawl", SKY_TYPE_GIANT, "Undead" },
        { 115, "Fright Rider", SKY_TYPE_SKYLANDER, "Undead" },

        // ==================== Magic Items ====================
        { 200, "Anvil Rain", SKY_TYPE_ITEM, "None" },
        { 201, "Hidden Treasure", SKY_TYPE_ITEM, "None" },
        { 202, "Healing Elixir", SKY_TYPE_ITEM, "None" },
        { 203, "Ghost Pirate Swords", SKY_TYPE_ITEM, "None" },
        { 204, "Time Twister Hourglass", SKY_TYPE_ITEM, "None" },
        { 205, "Sky-Iron Shield", SKY_TYPE_ITEM, "None" },
        { 206, "Winged Boots", SKY_TYPE_ITEM, "None" },
        { 207, "Sparx the Dragonfly", SKY_TYPE_ITEM, "None" },
        { 208, "Dragonfire Cannon", SKY_TYPE_ITEM, "None" },
        { 209, "Scorpion Striker Catapult", SKY_TYPE_ITEM, "None" },
        { 220, "Kaos Trap", SKY_TYPE_TRAP, "Kaos" },
        { 230, "Hand of Fate", SKY_TYPE_ITEM, "None" },
        { 231, "Piggy Bank", SKY_TYPE_ITEM, "None" },
        { 232, "Rocket Ram", SKY_TYPE_ITEM, "None" },
        { 233, "Tiki Speaky", SKY_TYPE_ITEM, "None" },
        { 235, "Mystery Chest", SKY_TYPE_ITEM, "None" },

        // ==================== Adventure Packs ====================
        { 300, "Dragon's Peak", SKY_TYPE_ITEM, "None" },
        { 301, "Empire of Ice", SKY_TYPE_ITEM, "None" },
        { 302, "Pirate Seas", SKY_TYPE_ITEM, "None" },
        { 303, "Darklight Crypt", SKY_TYPE_ITEM, "None" },
        { 304, "Volcanic Vault", SKY_TYPE_ITEM, "None" },
        { 305, "Mirror of Mystery", SKY_TYPE_ITEM, "None" },
        { 306, "Nightmare Express", SKY_TYPE_ITEM, "None" },
        { 307, "Sunscraper Spire", SKY_TYPE_ITEM, "None" },
        { 308, "Midnight Museum", SKY_TYPE_ITEM, "None" },
        { 310, "Gryphon Park Observatory", SKY_TYPE_ITEM, "None" },
        { 311, "Enchanted Elven Forest", SKY_TYPE_ITEM, "None" },

        // ==================== Legendary Variants ====================
        { 404, "Legendary Bash", SKY_TYPE_SKYLANDER, "Earth" },
        { 416, "Legendary Spyro", SKY_TYPE_SKYLANDER, "Magic" },
        { 419, "Legendary Trigger Happy", SKY_TYPE_SKYLANDER, "Tech" },
        { 430, "Legendary Chop Chop", SKY_TYPE_SKYLANDER, "Undead" },

        // ==================== Trap Team (2014) ====================
        { 450, "Gusto", SKY_TYPE_TRAP_MASTER, "Air" },
        { 451, "Thunderbolt", SKY_TYPE_TRAP_MASTER, "Air" },
        { 452, "Fling Kong", SKY_TYPE_SKYLANDER, "Air" },
        { 453, "Blades", SKY_TYPE_SKYLANDER, "Air" },
        { 454, "Wallop", SKY_TYPE_TRAP_MASTER, "Earth" },
        { 455, "Head Rush", SKY_TYPE_TRAP_MASTER, "Earth" },
        { 456, "Fist Bump", SKY_TYPE_SKYLANDER, "Earth" },
        { 457, "Rocky Roll", SKY_TYPE_SKYLANDER, "Earth" },
        { 458, "Wildfire", SKY_TYPE_TRAP_MASTER, "Fire" },
        { 459, "Ka-Boom", SKY_TYPE_TRAP_MASTER, "Fire" },
        { 460, "Trail Blazer", SKY_TYPE_SKYLANDER, "Fire" },
        { 461, "Torch", SKY_TYPE_SKYLANDER, "Fire" },
        { 462, "Snap Shot", SKY_TYPE_TRAP_MASTER, "Water" },
        { 463, "Lob-Star", SKY_TYPE_TRAP_MASTER, "Water" },
        { 464, "Flip Wreck", SKY_TYPE_SKYLANDER, "Water" },
        { 465, "Echo", SKY_TYPE_SKYLANDER, "Water" },
        { 466, "Blastermind", SKY_TYPE_TRAP_MASTER, "Magic" },
        { 467, "Enigma", SKY_TYPE_TRAP_MASTER, "Magic" },
        { 468, "Deja Vu", SKY_TYPE_SKYLANDER, "Magic" },
        { 469, "Cobra Cadabra", SKY_TYPE_SKYLANDER, "Magic" },
        { 470, "Jawbreaker", SKY_TYPE_TRAP_MASTER, "Tech" },
        { 471, "Gearshift", SKY_TYPE_TRAP_MASTER, "Tech" },
        { 472, "Chopper", SKY_TYPE_SKYLANDER, "Tech" },
        { 473, "Tread Head", SKY_TYPE_SKYLANDER, "Tech" },
        { 474, "Bushwhack", SKY_TYPE_TRAP_MASTER, "Life" },
        { 475, "Tuff Luck", SKY_TYPE_TRAP_MASTER, "Life" },
        { 476, "Food Fight", SKY_TYPE_SKYLANDER, "Life" },
        { 477, "High Five", SKY_TYPE_SKYLANDER, "Life" },
        { 478, "Krypt King", SKY_TYPE_TRAP_MASTER, "Undead" },
        { 479, "Short Cut", SKY_TYPE_TRAP_MASTER, "Undead" },
        { 480, "Bat Spin", SKY_TYPE_SKYLANDER, "Undead" },
        { 481, "Funny Bone", SKY_TYPE_SKYLANDER, "Undead" },
        { 482, "Knight Light", SKY_TYPE_TRAP_MASTER, "Light" },
        { 483, "Spotlight", SKY_TYPE_SKYLANDER, "Light" },
        { 484, "Knight Mare", SKY_TYPE_TRAP_MASTER, "Dark" },
        { 485, "Blackout", SKY_TYPE_SKYLANDER, "Dark" },

        // ==================== Minis ====================
        { 502, "Bop", SKY_TYPE_MINI, "Earth" },
        { 503, "Spry", SKY_TYPE_MINI, "Magic" },
        { 504, "Hijinx", SKY_TYPE_MINI, "Undead" },
        { 505, "Terrabite", SKY_TYPE_MINI, "Earth" },
        { 506, "Breeze", SKY_TYPE_MINI, "Air" },
        { 507, "Weeruptor", SKY_TYPE_MINI, "Fire" },
        { 508, "Pet-Vac", SKY_TYPE_MINI, "Air" },
        { 509, "Small Fry", SKY_TYPE_MINI, "Fire" },
        { 510, "Drobit", SKY_TYPE_MINI, "Tech" },
        { 511, "Gill Runt", SKY_TYPE_MINI, "Water" },
        { 519, "Trigger Snappy", SKY_TYPE_MINI, "Tech" },
        { 526, "Whisper Elf", SKY_TYPE_MINI, "Life" },
        { 540, "Barkley", SKY_TYPE_MINI, "Life" },
        { 541, "Thumpling", SKY_TYPE_MINI, "Water" },
        { 542, "Mini-Jini", SKY_TYPE_MINI, "Magic" },
        { 543, "Eye-Small", SKY_TYPE_MINI, "Undead" },

        // ==================== Imaginators Senseis (2016) ====================
        { 601, "King Pen", SKY_TYPE_IMAGINATOR, "Water" },
        { 602, "Tri-Tip", SKY_TYPE_IMAGINATOR, "Earth" },
        { 603, "Chopscotch", SKY_TYPE_IMAGINATOR, "Undead" },
        { 604, "Boom Bloom", SKY_TYPE_IMAGINATOR, "Life" },
        { 605, "Pit Boss", SKY_TYPE_IMAGINATOR, "Undead" },
        { 606, "Barbella", SKY_TYPE_IMAGINATOR, "Earth" },
        { 607, "Air Strike", SKY_TYPE_IMAGINATOR, "Air" },
        { 608, "Ember", SKY_TYPE_IMAGINATOR, "Fire" },
        { 609, "Ambush", SKY_TYPE_IMAGINATOR, "Life" },
        { 610, "Dr. Krankcase", SKY_TYPE_IMAGINATOR, "Tech" },
        { 611, "Hood Sickle", SKY_TYPE_IMAGINATOR, "Dark" },
        { 612, "Tae Kwon Crow", SKY_TYPE_IMAGINATOR, "Dark" },
        { 613, "Golden Queen", SKY_TYPE_IMAGINATOR, "Earth" },
        { 614, "Wolfgang", SKY_TYPE_IMAGINATOR, "Undead" },
        { 615, "Pain-Yatta", SKY_TYPE_IMAGINATOR, "Magic" },
        { 616, "Mysticat", SKY_TYPE_IMAGINATOR, "Magic" },
        { 617, "Starcast", SKY_TYPE_IMAGINATOR, "Dark" },
        { 618, "Buckshot", SKY_TYPE_IMAGINATOR, "Life" },
        { 619, "Aurora", SKY_TYPE_IMAGINATOR, "Light" },
        { 620, "Flare Wolf", SKY_TYPE_IMAGINATOR, "Fire" },
        { 621, "Chompy Mage", SKY_TYPE_IMAGINATOR, "Life" },
        { 622, "Bad Juju", SKY_TYPE_IMAGINATOR, "Air" },
        { 623, "Grave Clobber", SKY_TYPE_IMAGINATOR, "Earth" },
        { 624, "Blaster-Tron", SKY_TYPE_IMAGINATOR, "Tech" },
        { 625, "Ro-Bow", SKY_TYPE_IMAGINATOR, "Tech" },
        { 626, "Chain Reaction", SKY_TYPE_IMAGINATOR, "Tech" },
        { 627, "Kaos", SKY_TYPE_IMAGINATOR, "Kaos" },
        { 628, "Wild Storm", SKY_TYPE_IMAGINATOR, "Air" },
        { 629, "Tidepool", SKY_TYPE_IMAGINATOR, "Water" },
        { 630, "Crash Bandicoot", SKY_TYPE_IMAGINATOR, "Life" },
        { 631, "Dr. Neo Cortex", SKY_TYPE_IMAGINATOR, "Tech" },
        { 661, "Create Your Own Skylander", SKY_TYPE_CREATION_CRYSTAL, "None" },
        { 699, "Goldie", SKY_TYPE_IMAGINATOR, "None" },

        // ==================== Creation Crystals ====================
        { 680, "Magic Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, "Magic" },
        { 681, "Water Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, "Water" },
        { 682, "Air Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, "Air" },
        { 683, "Undead Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, "Undead" },
        { 684, "Tech Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, "Tech" },
        { 685, "Fire Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, "Fire" },
        { 686, "Earth Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, "Earth" },
        { 687, "Life Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, "Life" },
        { 688, "Dark Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, "Dark" },
        { 689, "Light Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, "Light" },

        // ==================== SWAP Force - Bottoms (2013) ====================
        { 1000, "Jet", SKY_TYPE_SWAP_BOTTOM, "Air" },
        { 1001, "Ranger", SKY_TYPE_SWAP_BOTTOM, "Air" },
        { 1002, "Rouser", SKY_TYPE_SWAP_BOTTOM, "Earth" },
        { 1003, "Stone", SKY_TYPE_SWAP_BOTTOM, "Earth" },
        { 1004, "Zone", SKY_TYPE_SWAP_BOTTOM, "Fire" },
        { 1005, "Kraken", SKY_TYPE_SWAP_BOTTOM, "Fire" },
        { 1006, "Bomb", SKY_TYPE_SWAP_BOTTOM, "Life" },
        { 1007, "Drilla", SKY_TYPE_SWAP_BOTTOM, "Life" },
        { 1008, "Loop", SKY_TYPE_SWAP_BOTTOM, "Magic" },
        { 1009, "Shadow", SKY_TYPE_SWAP_BOTTOM, "Magic" },
        { 1010, "Charge", SKY_TYPE_SWAP_BOTTOM, "Tech" },
        { 1011, "Rise", SKY_TYPE_SWAP_BOTTOM, "Tech" },
        { 1012, "Shift", SKY_TYPE_SWAP_BOTTOM, "Undead" },
        { 1013, "Shake", SKY_TYPE_SWAP_BOTTOM, "Undead" },
        { 1014, "Blade", SKY_TYPE_SWAP_BOTTOM, "Water" },
        { 1015, "Buckler", SKY_TYPE_SWAP_BOTTOM, "Water" },

        // ==================== SWAP Force - Tops (2013) ====================
        { 2000, "Boom", SKY_TYPE_SWAP_TOP, "Air" },
        { 2001, "Free", SKY_TYPE_SWAP_TOP, "Air" },
        { 2002, "Rubble", SKY_TYPE_SWAP_TOP, "Earth" },
        { 2003, "Doom", SKY_TYPE_SWAP_TOP, "Earth" },
        { 2004, "Blast", SKY_TYPE_SWAP_TOP, "Fire" },
        { 2005, "Fire", SKY_TYPE_SWAP_TOP, "Fire" },
        { 2006, "Stink", SKY_TYPE_SWAP_TOP, "Life" },
        { 2007, "Grilla", SKY_TYPE_SWAP_TOP, "Life" },
        { 2008, "Hoot", SKY_TYPE_SWAP_TOP, "Magic" },
        { 2009, "Trap", SKY_TYPE_SWAP_TOP, "Magic" },
        { 2010, "Magna", SKY_TYPE_SWAP_TOP, "Tech" },
        { 2011, "Spy", SKY_TYPE_SWAP_TOP, "Tech" },
        { 2012, "Night", SKY_TYPE_SWAP_TOP, "Undead" },
        { 2013, "Rattle", SKY_TYPE_SWAP_TOP, "Undead" },
        { 2014, "Freeze", SKY_TYPE_SWAP_TOP, "Water" },
        { 2015, "Wash", SKY_TYPE_SWAP_TOP, "Water" },

        // ==================== SWAP Force - Regular ====================
        { 3000, "Scratch", SKY_TYPE_SKYLANDER, "Air" },
        { 3001, "Pop Thorn", SKY_TYPE_SKYLANDER, "Air" },
        { 3002, "Slobber Tooth", SKY_TYPE_SKYLANDER, "Earth" },
        { 3003, "Scorp", SKY_TYPE_SKYLANDER, "Earth" },
        { 3004, "Fryno", SKY_TYPE_SKYLANDER, "Fire" },
        { 3005, "Smolderdash", SKY_TYPE_SKYLANDER, "Fire" },
        { 3006, "Bumble Blast", SKY_TYPE_SKYLANDER, "Life" },
        { 3007, "Zoo Lou", SKY_TYPE_SKYLANDER, "Life" },
        { 3008, "Dune Bug", SKY_TYPE_SKYLANDER, "Magic" },
        { 3009, "Star Strike", SKY_TYPE_SKYLANDER, "Magic" },
        { 3010, "Countdown", SKY_TYPE_SKYLANDER, "Tech" },
        { 3011, "Wind-Up", SKY_TYPE_SKYLANDER, "Tech" },
        { 3012, "Roller Brawl", SKY_TYPE_SKYLANDER, "Undead" },
        { 3013, "Grim Creeper", SKY_TYPE_SKYLANDER, "Undead" },
        { 3014, "Rip Tide", SKY_TYPE_SKYLANDER, "Water" },
        { 3015, "Punk Shock", SKY_TYPE_SKYLANDER, "Water" },

        // ==================== Racing Items ====================
        { 3200, "Battle Hammer", SKY_TYPE_ITEM, "None" },
        { 3201, "Sky Diamond", SKY_TYPE_ITEM, "None" },
        { 3202, "Platinum Sheep", SKY_TYPE_ITEM, "None" },
        { 3203, "Groove Machine", SKY_TYPE_ITEM, "None" },
        { 3204, "UFO Hat", SKY_TYPE_ITEM, "None" },

        // ==================== SuperChargers - Vehicles ====================
        { 3220, "Jet Stream", SKY_TYPE_VEHICLE, "Air" },
        { 3221, "Tomb Buggy", SKY_TYPE_VEHICLE, "Undead" },
        { 3222, "Reef Ripper", SKY_TYPE_VEHICLE, "Water" },
        { 3223, "Burn-Cycle", SKY_TYPE_VEHICLE, "Fire" },
        { 3224, "Hot Streak", SKY_TYPE_VEHICLE, "Fire" },
        { 3225, "Shark Tank", SKY_TYPE_VEHICLE, "Earth" },
        { 3226, "Thump Truck", SKY_TYPE_VEHICLE, "Earth" },
        { 3227, "Crypt Crusher", SKY_TYPE_VEHICLE, "Undead" },
        { 3228, "Stealth Stinger", SKY_TYPE_VEHICLE, "Life" },
        { 3231, "Dive Bomber", SKY_TYPE_VEHICLE, "Water" },
        { 3232, "Sky Slicer", SKY_TYPE_VEHICLE, "Air" },
        { 3233, "Clown Cruiser", SKY_TYPE_VEHICLE, "Fire" },
        { 3234, "Gold Rusher", SKY_TYPE_VEHICLE, "Tech" },
        { 3235, "Shield Striker", SKY_TYPE_VEHICLE, "Tech" },
        { 3236, "Sun Runner", SKY_TYPE_VEHICLE, "Light" },
        { 3237, "Sea Shadow", SKY_TYPE_VEHICLE, "Dark" },
        { 3238, "Splatter Splasher", SKY_TYPE_VEHICLE, "Magic" },
        { 3239, "Soda Skimmer", SKY_TYPE_VEHICLE, "Magic" },
        { 3240, "Barrel Blaster", SKY_TYPE_VEHICLE, "Life" },
        { 3241, "Buzz Wing", SKY_TYPE_VEHICLE, "Life" },

        // ==================== Adventure Packs (SWAP Force) ====================
        { 3300, "Sheep Wreck Island", SKY_TYPE_ITEM, "None" },
        { 3301, "Tower of Time", SKY_TYPE_ITEM, "None" },
        { 3302, "Arkeyan Crossbow", SKY_TYPE_ITEM, "None" },

        // ==================== SuperChargers - Drivers ====================
        { 3400, "Fiesta", SKY_TYPE_SUPERCHARGER, "Undead" },
        { 3401, "High Volt", SKY_TYPE_SUPERCHARGER, "Tech" },
        { 3402, "Splat", SKY_TYPE_SUPERCHARGER, "Magic" },
        { 3406, "Stormblade", SKY_TYPE_SUPERCHARGER, "Air" },
        { 3411, "Smash Hit", SKY_TYPE_SUPERCHARGER, "Earth" },
        { 3412, "Steel Plated Smash Hit", SKY_TYPE_SUPERCHARGER, "Earth" },
        { 3413, "Hurricane Jet-Vac", SKY_TYPE_SUPERCHARGER, "Air" },
        { 3414, "Double Dare Trigger Happy", SKY_TYPE_SUPERCHARGER, "Tech" },
        { 3415, "Super Shot Stealth Elf", SKY_TYPE_SUPERCHARGER, "Life" },
        { 3416, "Shark Shooter Terrafin", SKY_TYPE_SUPERCHARGER, "Earth" },
        { 3417, "Bone Bash Roller Brawl", SKY_TYPE_SUPERCHARGER, "Undead" },
        { 3420, "Big Bubble Pop Fizz", SKY_TYPE_SUPERCHARGER, "Magic" },
        { 3421, "Lava Lance Eruptor", SKY_TYPE_SUPERCHARGER, "Fire" },
        { 3422, "Deep Dive Gill Grunt", SKY_TYPE_SUPERCHARGER, "Water" },
        { 3423, "Turbo Charge Donkey Kong", SKY_TYPE_SUPERCHARGER, "Life" },
        { 3424, "Hammer Slam Bowser", SKY_TYPE_SUPERCHARGER, "Fire" },
        { 3425, "Dive-Clops", SKY_TYPE_SUPERCHARGER, "Water" },
        { 3426, "Astroblast", SKY_TYPE_SUPERCHARGER, "Light" },
        { 3427, "Nightfall", SKY_TYPE_SUPERCHARGER, "Dark" },
        { 3428, "Thrillipede", SKY_TYPE_SUPERCHARGER, "Life" },

        // ==================== Racing Villains ====================
        { 3440, "Chef Pepper Jack", SKY_TYPE_VEHICLE, "None" },
        { 3441, "Stratosfear", SKY_TYPE_VEHICLE, "None" },
        { 3442, "Cap'n Cluck", SKY_TYPE_VEHICLE, "None" },
        { 3443, "Wolfgang (Racing)", SKY_TYPE_VEHICLE, "None" },
        { 3444, "Threatpack", SKY_TYPE_VEHICLE, "None" },
        { 3445, "Hood Sickle (Racing)", SKY_TYPE_VEHICLE, "None" },
        { 3446, "Bad Juju (Racing)", SKY_TYPE_VEHICLE, "None" },

        // ==================== Trophies ====================
        { 3500, "Sky Trophy", SKY_TYPE_ITEM, "None" },
        { 3501, "Land Trophy", SKY_TYPE_ITEM, "None" },
        { 3502, "Sea Trophy", SKY_TYPE_ITEM, "None" },
        { 3503, "Kaos Trophy", SKY_TYPE_ITEM, "None" },

        // End marker
        { 0xFFFF, nullptr, SKY_TYPE_UNKNOWN, nullptr }
    };

    // Database size (excluding end marker)
    static const size_t DATABASE_SIZE = sizeof(SKYLANDER_DATABASE) / sizeof(SkylanderInfo) - 1;

    std::string skylanderDB::getName(uint16_t characterId)
    {
        const SkylanderInfo* info = getInfo(characterId);
        if (info)
            return info->name;

        // Unknown character - return ID as hex
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Unknown (ID: %d / 0x%04X)", characterId, characterId);
        return buffer;
    }

    const SkylanderInfo* skylanderDB::getInfo(uint16_t characterId)
    {
        for (size_t i = 0; i < DATABASE_SIZE; i++)
        {
            if (SKYLANDER_DATABASE[i].id == characterId)
                return &SKYLANDER_DATABASE[i];
        }
        return nullptr;
    }

    const char* skylanderDB::getTypeString(SkylanderType type)
    {
        switch (type)
        {
            case SKY_TYPE_SKYLANDER:        return "Skylander";
            case SKY_TYPE_GIANT:            return "Giant";
            case SKY_TYPE_SWAP_TOP:         return "Swapper Top";
            case SKY_TYPE_SWAP_BOTTOM:      return "Swapper Bottom";
            case SKY_TYPE_TRAP_MASTER:      return "Trap Master";
            case SKY_TYPE_MINI:             return "Mini";
            case SKY_TYPE_SUPERCHARGER:     return "SuperCharger";
            case SKY_TYPE_VEHICLE:          return "Vehicle";
            case SKY_TYPE_TRAP:             return "Trap";
            case SKY_TYPE_ITEM:             return "Magic Item";
            case SKY_TYPE_IMAGINATOR:       return "Sensei";
            case SKY_TYPE_CREATION_CRYSTAL: return "Creation Crystal";
            default:                        return "Unknown";
        }
    }

    uint16_t skylanderDB::getCharacterId(const uint8_t* data)
    {
        // Character ID is at offset 0x10 (little-endian)
        return data[SKY_CHARACTER_ID_OFFSET] | (data[SKY_CHARACTER_ID_OFFSET + 1] << 8);
    }

    uint16_t skylanderDB::getVariantId(const uint8_t* data)
    {
        // Variant ID is at offset 0x1C (little-endian)
        return data[SKY_VARIANT_ID_OFFSET] | (data[SKY_VARIANT_ID_OFFSET + 1] << 8);
    }

    bool skylanderDB::isCreationCrystal(uint16_t characterId)
    {
        return (characterId >= 680 && characterId <= 689) || characterId == 661;
    }

    bool skylanderDB::isTrap(uint16_t characterId)
    {
        return characterId == 220; // Kaos Trap (element traps use different system)
    }

    bool skylanderDB::isVehicle(uint16_t characterId)
    {
        return (characterId >= 3220 && characterId <= 3241);
    }
}
