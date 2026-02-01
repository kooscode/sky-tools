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
    // Format: { ID, "Name", Type, Element, Game }
    static const SkylanderInfo SKYLANDER_DATABASE[] =
    {
        // ==================== Spyro's Adventure (2011) ====================
        { 0, "Whirlwind", SKY_TYPE_SKYLANDER, SKY_ELEMENT_AIR, SKY_GAME_SPYROS_ADVENTURE },
        { 1, "Sonic Boom", SKY_TYPE_SKYLANDER, SKY_ELEMENT_AIR, SKY_GAME_SPYROS_ADVENTURE },
        { 2, "Warnado", SKY_TYPE_SKYLANDER, SKY_ELEMENT_AIR, SKY_GAME_SPYROS_ADVENTURE },
        { 3, "Lightning Rod", SKY_TYPE_SKYLANDER, SKY_ELEMENT_AIR, SKY_GAME_SPYROS_ADVENTURE },
        { 4, "Bash", SKY_TYPE_SKYLANDER, SKY_ELEMENT_EARTH, SKY_GAME_SPYROS_ADVENTURE },
        { 5, "Terrafin", SKY_TYPE_SKYLANDER, SKY_ELEMENT_EARTH, SKY_GAME_SPYROS_ADVENTURE },
        { 6, "Dino-Rang", SKY_TYPE_SKYLANDER, SKY_ELEMENT_EARTH, SKY_GAME_SPYROS_ADVENTURE },
        { 7, "Prism Break", SKY_TYPE_SKYLANDER, SKY_ELEMENT_EARTH, SKY_GAME_SPYROS_ADVENTURE },
        { 8, "Sunburn", SKY_TYPE_SKYLANDER, SKY_ELEMENT_FIRE, SKY_GAME_SPYROS_ADVENTURE },
        { 9, "Eruptor", SKY_TYPE_SKYLANDER, SKY_ELEMENT_FIRE, SKY_GAME_SPYROS_ADVENTURE },
        { 10, "Ignitor", SKY_TYPE_SKYLANDER, SKY_ELEMENT_FIRE, SKY_GAME_SPYROS_ADVENTURE },
        { 11, "Flameslinger", SKY_TYPE_SKYLANDER, SKY_ELEMENT_FIRE, SKY_GAME_SPYROS_ADVENTURE },
        { 12, "Zap", SKY_TYPE_SKYLANDER, SKY_ELEMENT_WATER, SKY_GAME_SPYROS_ADVENTURE },
        { 13, "Wham-Shell", SKY_TYPE_SKYLANDER, SKY_ELEMENT_WATER, SKY_GAME_SPYROS_ADVENTURE },
        { 14, "Gill Grunt", SKY_TYPE_SKYLANDER, SKY_ELEMENT_WATER, SKY_GAME_SPYROS_ADVENTURE },
        { 15, "Slam Bam", SKY_TYPE_SKYLANDER, SKY_ELEMENT_WATER, SKY_GAME_SPYROS_ADVENTURE },
        { 16, "Spyro", SKY_TYPE_SKYLANDER, SKY_ELEMENT_MAGIC, SKY_GAME_SPYROS_ADVENTURE },
        { 17, "Voodood", SKY_TYPE_SKYLANDER, SKY_ELEMENT_MAGIC, SKY_GAME_SPYROS_ADVENTURE },
        { 18, "Double Trouble", SKY_TYPE_SKYLANDER, SKY_ELEMENT_MAGIC, SKY_GAME_SPYROS_ADVENTURE },
        { 19, "Trigger Happy", SKY_TYPE_SKYLANDER, SKY_ELEMENT_TECH, SKY_GAME_SPYROS_ADVENTURE },
        { 20, "Drobot", SKY_TYPE_SKYLANDER, SKY_ELEMENT_TECH, SKY_GAME_SPYROS_ADVENTURE },
        { 21, "Drill Sergeant", SKY_TYPE_SKYLANDER, SKY_ELEMENT_TECH, SKY_GAME_SPYROS_ADVENTURE },
        { 22, "Boomer", SKY_TYPE_SKYLANDER, SKY_ELEMENT_TECH, SKY_GAME_SPYROS_ADVENTURE },
        { 23, "Wrecking Ball", SKY_TYPE_SKYLANDER, SKY_ELEMENT_MAGIC, SKY_GAME_SPYROS_ADVENTURE },
        { 24, "Camo", SKY_TYPE_SKYLANDER, SKY_ELEMENT_LIFE, SKY_GAME_SPYROS_ADVENTURE },
        { 25, "Zook", SKY_TYPE_SKYLANDER, SKY_ELEMENT_LIFE, SKY_GAME_SPYROS_ADVENTURE },
        { 26, "Stealth Elf", SKY_TYPE_SKYLANDER, SKY_ELEMENT_LIFE, SKY_GAME_SPYROS_ADVENTURE },
        { 27, "Stump Smash", SKY_TYPE_SKYLANDER, SKY_ELEMENT_LIFE, SKY_GAME_SPYROS_ADVENTURE },
        { 28, "Dark Spyro", SKY_TYPE_SKYLANDER, SKY_ELEMENT_MAGIC, SKY_GAME_SPYROS_ADVENTURE },
        { 29, "Hex", SKY_TYPE_SKYLANDER, SKY_ELEMENT_UNDEAD, SKY_GAME_SPYROS_ADVENTURE },
        { 30, "Chop Chop", SKY_TYPE_SKYLANDER, SKY_ELEMENT_UNDEAD, SKY_GAME_SPYROS_ADVENTURE },
        { 31, "Ghost Roaster", SKY_TYPE_SKYLANDER, SKY_ELEMENT_UNDEAD, SKY_GAME_SPYROS_ADVENTURE },
        { 32, "Cynder", SKY_TYPE_SKYLANDER, SKY_ELEMENT_UNDEAD, SKY_GAME_SPYROS_ADVENTURE },

        // ==================== Giants (2012) ====================
        { 100, "Jet-Vac", SKY_TYPE_SKYLANDER, SKY_ELEMENT_AIR, SKY_GAME_GIANTS },
        { 101, "Swarm", SKY_TYPE_GIANT, SKY_ELEMENT_AIR, SKY_GAME_GIANTS },
        { 102, "Crusher", SKY_TYPE_GIANT, SKY_ELEMENT_EARTH, SKY_GAME_GIANTS },
        { 103, "Flashwing", SKY_TYPE_SKYLANDER, SKY_ELEMENT_EARTH, SKY_GAME_GIANTS },
        { 104, "Hot Head", SKY_TYPE_GIANT, SKY_ELEMENT_FIRE, SKY_GAME_GIANTS },
        { 105, "Hot Dog", SKY_TYPE_SKYLANDER, SKY_ELEMENT_FIRE, SKY_GAME_GIANTS },
        { 106, "Chill", SKY_TYPE_SKYLANDER, SKY_ELEMENT_WATER, SKY_GAME_GIANTS },
        { 107, "Thumpback", SKY_TYPE_GIANT, SKY_ELEMENT_WATER, SKY_GAME_GIANTS },
        { 108, "Pop Fizz", SKY_TYPE_SKYLANDER, SKY_ELEMENT_MAGIC, SKY_GAME_GIANTS },
        { 109, "Ninjini", SKY_TYPE_GIANT, SKY_ELEMENT_MAGIC, SKY_GAME_GIANTS },
        { 110, "Bouncer", SKY_TYPE_GIANT, SKY_ELEMENT_TECH, SKY_GAME_GIANTS },
        { 111, "Sprocket", SKY_TYPE_SKYLANDER, SKY_ELEMENT_TECH, SKY_GAME_GIANTS },
        { 112, "Tree Rex", SKY_TYPE_GIANT, SKY_ELEMENT_LIFE, SKY_GAME_GIANTS },
        { 113, "Shroomboom", SKY_TYPE_SKYLANDER, SKY_ELEMENT_LIFE, SKY_GAME_GIANTS },
        { 114, "Eye-Brawl", SKY_TYPE_GIANT, SKY_ELEMENT_UNDEAD, SKY_GAME_GIANTS },
        { 115, "Fright Rider", SKY_TYPE_SKYLANDER, SKY_ELEMENT_UNDEAD, SKY_GAME_GIANTS },

        // ==================== Magic Items ====================
        { 200, "Anvil Rain", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SPYROS_ADVENTURE },
        { 201, "Hidden Treasure", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SPYROS_ADVENTURE },
        { 202, "Healing Elixir", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SPYROS_ADVENTURE },
        { 203, "Ghost Pirate Swords", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SPYROS_ADVENTURE },
        { 204, "Time Twister Hourglass", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SPYROS_ADVENTURE },
        { 205, "Sky-Iron Shield", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SPYROS_ADVENTURE },
        { 206, "Winged Boots", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SPYROS_ADVENTURE },
        { 207, "Sparx the Dragonfly", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SPYROS_ADVENTURE },
        { 208, "Dragonfire Cannon", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_GIANTS },
        { 209, "Scorpion Striker Catapult", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_GIANTS },
        { 220, "Kaos Trap", SKY_TYPE_TRAP, SKY_ELEMENT_KAOS, SKY_GAME_TRAP_TEAM },
        { 230, "Hand of Fate", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_TRAP_TEAM },
        { 231, "Piggy Bank", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_TRAP_TEAM },
        { 232, "Rocket Ram", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_TRAP_TEAM },
        { 233, "Tiki Speaky", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_TRAP_TEAM },
        { 235, "Mystery Chest", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_TRAP_TEAM },

        // ==================== Adventure Packs ====================
        { 300, "Dragon's Peak", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SPYROS_ADVENTURE },
        { 301, "Empire of Ice", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SPYROS_ADVENTURE },
        { 302, "Pirate Seas", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SPYROS_ADVENTURE },
        { 303, "Darklight Crypt", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SPYROS_ADVENTURE },
        { 304, "Volcanic Vault", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_GIANTS },
        { 305, "Mirror of Mystery", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_TRAP_TEAM },
        { 306, "Nightmare Express", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_TRAP_TEAM },
        { 307, "Sunscraper Spire", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_TRAP_TEAM },
        { 308, "Midnight Museum", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_TRAP_TEAM },
        { 310, "Gryphon Park Observatory", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },
        { 311, "Enchanted Elven Forest", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },

        // ==================== Legendary Variants ====================
        { 404, "Legendary Bash", SKY_TYPE_SKYLANDER, SKY_ELEMENT_EARTH, SKY_GAME_SPYROS_ADVENTURE },
        { 416, "Legendary Spyro", SKY_TYPE_SKYLANDER, SKY_ELEMENT_MAGIC, SKY_GAME_SPYROS_ADVENTURE },
        { 419, "Legendary Trigger Happy", SKY_TYPE_SKYLANDER, SKY_ELEMENT_TECH, SKY_GAME_SPYROS_ADVENTURE },
        { 430, "Legendary Chop Chop", SKY_TYPE_SKYLANDER, SKY_ELEMENT_UNDEAD, SKY_GAME_SPYROS_ADVENTURE },

        // ==================== Trap Team (2014) ====================
        { 450, "Gusto", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_AIR, SKY_GAME_TRAP_TEAM },
        { 451, "Thunderbolt", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_AIR, SKY_GAME_TRAP_TEAM },
        { 452, "Fling Kong", SKY_TYPE_SKYLANDER, SKY_ELEMENT_AIR, SKY_GAME_TRAP_TEAM },
        { 453, "Blades", SKY_TYPE_SKYLANDER, SKY_ELEMENT_AIR, SKY_GAME_TRAP_TEAM },
        { 454, "Wallop", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_EARTH, SKY_GAME_TRAP_TEAM },
        { 455, "Head Rush", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_EARTH, SKY_GAME_TRAP_TEAM },
        { 456, "Fist Bump", SKY_TYPE_SKYLANDER, SKY_ELEMENT_EARTH, SKY_GAME_TRAP_TEAM },
        { 457, "Rocky Roll", SKY_TYPE_SKYLANDER, SKY_ELEMENT_EARTH, SKY_GAME_TRAP_TEAM },
        { 458, "Wildfire", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_FIRE, SKY_GAME_TRAP_TEAM },
        { 459, "Ka-Boom", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_FIRE, SKY_GAME_TRAP_TEAM },
        { 460, "Trail Blazer", SKY_TYPE_SKYLANDER, SKY_ELEMENT_FIRE, SKY_GAME_TRAP_TEAM },
        { 461, "Torch", SKY_TYPE_SKYLANDER, SKY_ELEMENT_FIRE, SKY_GAME_TRAP_TEAM },
        { 462, "Snap Shot", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_WATER, SKY_GAME_TRAP_TEAM },
        { 463, "Lob-Star", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_WATER, SKY_GAME_TRAP_TEAM },
        { 464, "Flip Wreck", SKY_TYPE_SKYLANDER, SKY_ELEMENT_WATER, SKY_GAME_TRAP_TEAM },
        { 465, "Echo", SKY_TYPE_SKYLANDER, SKY_ELEMENT_WATER, SKY_GAME_TRAP_TEAM },
        { 466, "Blastermind", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_MAGIC, SKY_GAME_TRAP_TEAM },
        { 467, "Enigma", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_MAGIC, SKY_GAME_TRAP_TEAM },
        { 468, "Deja Vu", SKY_TYPE_SKYLANDER, SKY_ELEMENT_MAGIC, SKY_GAME_TRAP_TEAM },
        { 469, "Cobra Cadabra", SKY_TYPE_SKYLANDER, SKY_ELEMENT_MAGIC, SKY_GAME_TRAP_TEAM },
        { 470, "Jawbreaker", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_TECH, SKY_GAME_TRAP_TEAM },
        { 471, "Gearshift", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_TECH, SKY_GAME_TRAP_TEAM },
        { 472, "Chopper", SKY_TYPE_SKYLANDER, SKY_ELEMENT_TECH, SKY_GAME_TRAP_TEAM },
        { 473, "Tread Head", SKY_TYPE_SKYLANDER, SKY_ELEMENT_TECH, SKY_GAME_TRAP_TEAM },
        { 474, "Bushwhack", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_LIFE, SKY_GAME_TRAP_TEAM },
        { 475, "Tuff Luck", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_LIFE, SKY_GAME_TRAP_TEAM },
        { 476, "Food Fight", SKY_TYPE_SKYLANDER, SKY_ELEMENT_LIFE, SKY_GAME_TRAP_TEAM },
        { 477, "High Five", SKY_TYPE_SKYLANDER, SKY_ELEMENT_LIFE, SKY_GAME_TRAP_TEAM },
        { 478, "Krypt King", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_UNDEAD, SKY_GAME_TRAP_TEAM },
        { 479, "Short Cut", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_UNDEAD, SKY_GAME_TRAP_TEAM },
        { 480, "Bat Spin", SKY_TYPE_SKYLANDER, SKY_ELEMENT_UNDEAD, SKY_GAME_TRAP_TEAM },
        { 481, "Funny Bone", SKY_TYPE_SKYLANDER, SKY_ELEMENT_UNDEAD, SKY_GAME_TRAP_TEAM },
        { 482, "Knight Light", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_LIGHT, SKY_GAME_TRAP_TEAM },
        { 483, "Spotlight", SKY_TYPE_SKYLANDER, SKY_ELEMENT_LIGHT, SKY_GAME_TRAP_TEAM },
        { 484, "Knight Mare", SKY_TYPE_TRAP_MASTER, SKY_ELEMENT_DARK, SKY_GAME_TRAP_TEAM },
        { 485, "Blackout", SKY_TYPE_SKYLANDER, SKY_ELEMENT_DARK, SKY_GAME_TRAP_TEAM },

        // ==================== Minis ====================
        { 502, "Bop", SKY_TYPE_MINI, SKY_ELEMENT_EARTH, SKY_GAME_TRAP_TEAM },
        { 503, "Spry", SKY_TYPE_MINI, SKY_ELEMENT_MAGIC, SKY_GAME_TRAP_TEAM },
        { 504, "Hijinx", SKY_TYPE_MINI, SKY_ELEMENT_UNDEAD, SKY_GAME_TRAP_TEAM },
        { 505, "Terrabite", SKY_TYPE_MINI, SKY_ELEMENT_EARTH, SKY_GAME_TRAP_TEAM },
        { 506, "Breeze", SKY_TYPE_MINI, SKY_ELEMENT_AIR, SKY_GAME_TRAP_TEAM },
        { 507, "Weeruptor", SKY_TYPE_MINI, SKY_ELEMENT_FIRE, SKY_GAME_TRAP_TEAM },
        { 508, "Pet-Vac", SKY_TYPE_MINI, SKY_ELEMENT_AIR, SKY_GAME_TRAP_TEAM },
        { 509, "Small Fry", SKY_TYPE_MINI, SKY_ELEMENT_FIRE, SKY_GAME_TRAP_TEAM },
        { 510, "Drobit", SKY_TYPE_MINI, SKY_ELEMENT_TECH, SKY_GAME_TRAP_TEAM },
        { 511, "Gill Runt", SKY_TYPE_MINI, SKY_ELEMENT_WATER, SKY_GAME_TRAP_TEAM },
        { 519, "Trigger Snappy", SKY_TYPE_MINI, SKY_ELEMENT_TECH, SKY_GAME_TRAP_TEAM },
        { 526, "Whisper Elf", SKY_TYPE_MINI, SKY_ELEMENT_LIFE, SKY_GAME_TRAP_TEAM },
        { 540, "Barkley", SKY_TYPE_MINI, SKY_ELEMENT_LIFE, SKY_GAME_TRAP_TEAM },
        { 541, "Thumpling", SKY_TYPE_MINI, SKY_ELEMENT_WATER, SKY_GAME_TRAP_TEAM },
        { 542, "Mini-Jini", SKY_TYPE_MINI, SKY_ELEMENT_MAGIC, SKY_GAME_TRAP_TEAM },
        { 543, "Eye-Small", SKY_TYPE_MINI, SKY_ELEMENT_UNDEAD, SKY_GAME_TRAP_TEAM },

        // ==================== Imaginators Senseis (2016) ====================
        { 601, "King Pen", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_WATER, SKY_GAME_IMAGINATORS },
        { 602, "Tri-Tip", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_EARTH, SKY_GAME_IMAGINATORS },
        { 603, "Chopscotch", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_UNDEAD, SKY_GAME_IMAGINATORS },
        { 604, "Boom Bloom", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_LIFE, SKY_GAME_IMAGINATORS },
        { 605, "Pit Boss", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_UNDEAD, SKY_GAME_IMAGINATORS },
        { 606, "Barbella", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_EARTH, SKY_GAME_IMAGINATORS },
        { 607, "Air Strike", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_AIR, SKY_GAME_IMAGINATORS },
        { 608, "Ember", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_FIRE, SKY_GAME_IMAGINATORS },
        { 609, "Ambush", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_LIFE, SKY_GAME_IMAGINATORS },
        { 610, "Dr. Krankcase", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_TECH, SKY_GAME_IMAGINATORS },
        { 611, "Hood Sickle", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_DARK, SKY_GAME_IMAGINATORS },
        { 612, "Tae Kwon Crow", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_DARK, SKY_GAME_IMAGINATORS },
        { 613, "Golden Queen", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_EARTH, SKY_GAME_IMAGINATORS },
        { 614, "Wolfgang", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_UNDEAD, SKY_GAME_IMAGINATORS },
        { 615, "Pain-Yatta", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_MAGIC, SKY_GAME_IMAGINATORS },
        { 616, "Mysticat", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_MAGIC, SKY_GAME_IMAGINATORS },
        { 617, "Starcast", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_DARK, SKY_GAME_IMAGINATORS },
        { 618, "Buckshot", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_LIFE, SKY_GAME_IMAGINATORS },
        { 619, "Aurora", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_LIGHT, SKY_GAME_IMAGINATORS },
        { 620, "Flare Wolf", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_FIRE, SKY_GAME_IMAGINATORS },
        { 621, "Chompy Mage", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_LIFE, SKY_GAME_IMAGINATORS },
        { 622, "Bad Juju", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_AIR, SKY_GAME_IMAGINATORS },
        { 623, "Grave Clobber", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_EARTH, SKY_GAME_IMAGINATORS },
        { 624, "Blaster-Tron", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_TECH, SKY_GAME_IMAGINATORS },
        { 625, "Ro-Bow", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_TECH, SKY_GAME_IMAGINATORS },
        { 626, "Chain Reaction", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_TECH, SKY_GAME_IMAGINATORS },
        { 627, "Kaos", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_KAOS, SKY_GAME_IMAGINATORS },
        { 628, "Wild Storm", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_AIR, SKY_GAME_IMAGINATORS },
        { 629, "Tidepool", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_WATER, SKY_GAME_IMAGINATORS },
        { 630, "Crash Bandicoot", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_LIFE, SKY_GAME_IMAGINATORS },
        { 631, "Dr. Neo Cortex", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_TECH, SKY_GAME_IMAGINATORS },
        { 661, "Create Your Own Skylander", SKY_TYPE_CREATION_CRYSTAL, SKY_ELEMENT_NONE, SKY_GAME_IMAGINATORS },
        { 699, "Goldie", SKY_TYPE_IMAGINATOR, SKY_ELEMENT_NONE, SKY_GAME_IMAGINATORS },

        // ==================== Creation Crystals ====================
        { 680, "Magic Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, SKY_ELEMENT_MAGIC, SKY_GAME_IMAGINATORS },
        { 681, "Water Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, SKY_ELEMENT_WATER, SKY_GAME_IMAGINATORS },
        { 682, "Air Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, SKY_ELEMENT_AIR, SKY_GAME_IMAGINATORS },
        { 683, "Undead Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, SKY_ELEMENT_UNDEAD, SKY_GAME_IMAGINATORS },
        { 684, "Tech Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, SKY_ELEMENT_TECH, SKY_GAME_IMAGINATORS },
        { 685, "Fire Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, SKY_ELEMENT_FIRE, SKY_GAME_IMAGINATORS },
        { 686, "Earth Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, SKY_ELEMENT_EARTH, SKY_GAME_IMAGINATORS },
        { 687, "Life Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, SKY_ELEMENT_LIFE, SKY_GAME_IMAGINATORS },
        { 688, "Dark Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, SKY_ELEMENT_DARK, SKY_GAME_IMAGINATORS },
        { 689, "Light Creation Crystal", SKY_TYPE_CREATION_CRYSTAL, SKY_ELEMENT_LIGHT, SKY_GAME_IMAGINATORS },

        // ==================== SWAP Force - Bottoms (2013) ====================
        { 1000, "Jet", SKY_TYPE_SWAP_BOTTOM, SKY_ELEMENT_AIR, SKY_GAME_SWAP_FORCE },
        { 1001, "Ranger", SKY_TYPE_SWAP_BOTTOM, SKY_ELEMENT_AIR, SKY_GAME_SWAP_FORCE },
        { 1002, "Rouser", SKY_TYPE_SWAP_BOTTOM, SKY_ELEMENT_EARTH, SKY_GAME_SWAP_FORCE },
        { 1003, "Stone", SKY_TYPE_SWAP_BOTTOM, SKY_ELEMENT_EARTH, SKY_GAME_SWAP_FORCE },
        { 1004, "Zone", SKY_TYPE_SWAP_BOTTOM, SKY_ELEMENT_FIRE, SKY_GAME_SWAP_FORCE },
        { 1005, "Kraken", SKY_TYPE_SWAP_BOTTOM, SKY_ELEMENT_FIRE, SKY_GAME_SWAP_FORCE },
        { 1006, "Bomb", SKY_TYPE_SWAP_BOTTOM, SKY_ELEMENT_LIFE, SKY_GAME_SWAP_FORCE },
        { 1007, "Drilla", SKY_TYPE_SWAP_BOTTOM, SKY_ELEMENT_LIFE, SKY_GAME_SWAP_FORCE },
        { 1008, "Loop", SKY_TYPE_SWAP_BOTTOM, SKY_ELEMENT_MAGIC, SKY_GAME_SWAP_FORCE },
        { 1009, "Shadow", SKY_TYPE_SWAP_BOTTOM, SKY_ELEMENT_MAGIC, SKY_GAME_SWAP_FORCE },
        { 1010, "Charge", SKY_TYPE_SWAP_BOTTOM, SKY_ELEMENT_TECH, SKY_GAME_SWAP_FORCE },
        { 1011, "Rise", SKY_TYPE_SWAP_BOTTOM, SKY_ELEMENT_TECH, SKY_GAME_SWAP_FORCE },
        { 1012, "Shift", SKY_TYPE_SWAP_BOTTOM, SKY_ELEMENT_UNDEAD, SKY_GAME_SWAP_FORCE },
        { 1013, "Shake", SKY_TYPE_SWAP_BOTTOM, SKY_ELEMENT_UNDEAD, SKY_GAME_SWAP_FORCE },
        { 1014, "Blade", SKY_TYPE_SWAP_BOTTOM, SKY_ELEMENT_WATER, SKY_GAME_SWAP_FORCE },
        { 1015, "Buckler", SKY_TYPE_SWAP_BOTTOM, SKY_ELEMENT_WATER, SKY_GAME_SWAP_FORCE },

        // ==================== SWAP Force - Tops (2013) ====================
        { 2000, "Boom", SKY_TYPE_SWAP_TOP, SKY_ELEMENT_AIR, SKY_GAME_SWAP_FORCE },
        { 2001, "Free", SKY_TYPE_SWAP_TOP, SKY_ELEMENT_AIR, SKY_GAME_SWAP_FORCE },
        { 2002, "Rubble", SKY_TYPE_SWAP_TOP, SKY_ELEMENT_EARTH, SKY_GAME_SWAP_FORCE },
        { 2003, "Doom", SKY_TYPE_SWAP_TOP, SKY_ELEMENT_EARTH, SKY_GAME_SWAP_FORCE },
        { 2004, "Blast", SKY_TYPE_SWAP_TOP, SKY_ELEMENT_FIRE, SKY_GAME_SWAP_FORCE },
        { 2005, "Fire", SKY_TYPE_SWAP_TOP, SKY_ELEMENT_FIRE, SKY_GAME_SWAP_FORCE },
        { 2006, "Stink", SKY_TYPE_SWAP_TOP, SKY_ELEMENT_LIFE, SKY_GAME_SWAP_FORCE },
        { 2007, "Grilla", SKY_TYPE_SWAP_TOP, SKY_ELEMENT_LIFE, SKY_GAME_SWAP_FORCE },
        { 2008, "Hoot", SKY_TYPE_SWAP_TOP, SKY_ELEMENT_MAGIC, SKY_GAME_SWAP_FORCE },
        { 2009, "Trap", SKY_TYPE_SWAP_TOP, SKY_ELEMENT_MAGIC, SKY_GAME_SWAP_FORCE },
        { 2010, "Magna", SKY_TYPE_SWAP_TOP, SKY_ELEMENT_TECH, SKY_GAME_SWAP_FORCE },
        { 2011, "Spy", SKY_TYPE_SWAP_TOP, SKY_ELEMENT_TECH, SKY_GAME_SWAP_FORCE },
        { 2012, "Night", SKY_TYPE_SWAP_TOP, SKY_ELEMENT_UNDEAD, SKY_GAME_SWAP_FORCE },
        { 2013, "Rattle", SKY_TYPE_SWAP_TOP, SKY_ELEMENT_UNDEAD, SKY_GAME_SWAP_FORCE },
        { 2014, "Freeze", SKY_TYPE_SWAP_TOP, SKY_ELEMENT_WATER, SKY_GAME_SWAP_FORCE },
        { 2015, "Wash", SKY_TYPE_SWAP_TOP, SKY_ELEMENT_WATER, SKY_GAME_SWAP_FORCE },

        // ==================== SWAP Force - Regular ====================
        { 3000, "Scratch", SKY_TYPE_SKYLANDER, SKY_ELEMENT_AIR, SKY_GAME_SWAP_FORCE },
        { 3001, "Pop Thorn", SKY_TYPE_SKYLANDER, SKY_ELEMENT_AIR, SKY_GAME_SWAP_FORCE },
        { 3002, "Slobber Tooth", SKY_TYPE_SKYLANDER, SKY_ELEMENT_EARTH, SKY_GAME_SWAP_FORCE },
        { 3003, "Scorp", SKY_TYPE_SKYLANDER, SKY_ELEMENT_EARTH, SKY_GAME_SWAP_FORCE },
        { 3004, "Fryno", SKY_TYPE_SKYLANDER, SKY_ELEMENT_FIRE, SKY_GAME_SWAP_FORCE },
        { 3005, "Smolderdash", SKY_TYPE_SKYLANDER, SKY_ELEMENT_FIRE, SKY_GAME_SWAP_FORCE },
        { 3006, "Bumble Blast", SKY_TYPE_SKYLANDER, SKY_ELEMENT_LIFE, SKY_GAME_SWAP_FORCE },
        { 3007, "Zoo Lou", SKY_TYPE_SKYLANDER, SKY_ELEMENT_LIFE, SKY_GAME_SWAP_FORCE },
        { 3008, "Dune Bug", SKY_TYPE_SKYLANDER, SKY_ELEMENT_MAGIC, SKY_GAME_SWAP_FORCE },
        { 3009, "Star Strike", SKY_TYPE_SKYLANDER, SKY_ELEMENT_MAGIC, SKY_GAME_SWAP_FORCE },
        { 3010, "Countdown", SKY_TYPE_SKYLANDER, SKY_ELEMENT_TECH, SKY_GAME_SWAP_FORCE },
        { 3011, "Wind-Up", SKY_TYPE_SKYLANDER, SKY_ELEMENT_TECH, SKY_GAME_SWAP_FORCE },
        { 3012, "Roller Brawl", SKY_TYPE_SKYLANDER, SKY_ELEMENT_UNDEAD, SKY_GAME_SWAP_FORCE },
        { 3013, "Grim Creeper", SKY_TYPE_SKYLANDER, SKY_ELEMENT_UNDEAD, SKY_GAME_SWAP_FORCE },
        { 3014, "Rip Tide", SKY_TYPE_SKYLANDER, SKY_ELEMENT_WATER, SKY_GAME_SWAP_FORCE },
        { 3015, "Punk Shock", SKY_TYPE_SKYLANDER, SKY_ELEMENT_WATER, SKY_GAME_SWAP_FORCE },

        // ==================== Racing Items ====================
        { 3200, "Battle Hammer", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },
        { 3201, "Sky Diamond", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },
        { 3202, "Platinum Sheep", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },
        { 3203, "Groove Machine", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },
        { 3204, "UFO Hat", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },

        // ==================== SuperChargers - Vehicles ====================
        { 3220, "Jet Stream", SKY_TYPE_VEHICLE, SKY_ELEMENT_AIR, SKY_GAME_SUPERCHARGERS },
        { 3221, "Tomb Buggy", SKY_TYPE_VEHICLE, SKY_ELEMENT_UNDEAD, SKY_GAME_SUPERCHARGERS },
        { 3222, "Reef Ripper", SKY_TYPE_VEHICLE, SKY_ELEMENT_WATER, SKY_GAME_SUPERCHARGERS },
        { 3223, "Burn-Cycle", SKY_TYPE_VEHICLE, SKY_ELEMENT_FIRE, SKY_GAME_SUPERCHARGERS },
        { 3224, "Hot Streak", SKY_TYPE_VEHICLE, SKY_ELEMENT_FIRE, SKY_GAME_SUPERCHARGERS },
        { 3225, "Shark Tank", SKY_TYPE_VEHICLE, SKY_ELEMENT_EARTH, SKY_GAME_SUPERCHARGERS },
        { 3226, "Thump Truck", SKY_TYPE_VEHICLE, SKY_ELEMENT_EARTH, SKY_GAME_SUPERCHARGERS },
        { 3227, "Crypt Crusher", SKY_TYPE_VEHICLE, SKY_ELEMENT_UNDEAD, SKY_GAME_SUPERCHARGERS },
        { 3228, "Stealth Stinger", SKY_TYPE_VEHICLE, SKY_ELEMENT_LIFE, SKY_GAME_SUPERCHARGERS },
        { 3231, "Dive Bomber", SKY_TYPE_VEHICLE, SKY_ELEMENT_WATER, SKY_GAME_SUPERCHARGERS },
        { 3232, "Sky Slicer", SKY_TYPE_VEHICLE, SKY_ELEMENT_AIR, SKY_GAME_SUPERCHARGERS },
        { 3233, "Clown Cruiser", SKY_TYPE_VEHICLE, SKY_ELEMENT_FIRE, SKY_GAME_SUPERCHARGERS },
        { 3234, "Gold Rusher", SKY_TYPE_VEHICLE, SKY_ELEMENT_TECH, SKY_GAME_SUPERCHARGERS },
        { 3235, "Shield Striker", SKY_TYPE_VEHICLE, SKY_ELEMENT_TECH, SKY_GAME_SUPERCHARGERS },
        { 3236, "Sun Runner", SKY_TYPE_VEHICLE, SKY_ELEMENT_LIGHT, SKY_GAME_SUPERCHARGERS },
        { 3237, "Sea Shadow", SKY_TYPE_VEHICLE, SKY_ELEMENT_DARK, SKY_GAME_SUPERCHARGERS },
        { 3238, "Splatter Splasher", SKY_TYPE_VEHICLE, SKY_ELEMENT_MAGIC, SKY_GAME_SUPERCHARGERS },
        { 3239, "Soda Skimmer", SKY_TYPE_VEHICLE, SKY_ELEMENT_MAGIC, SKY_GAME_SUPERCHARGERS },
        { 3240, "Barrel Blaster", SKY_TYPE_VEHICLE, SKY_ELEMENT_LIFE, SKY_GAME_SUPERCHARGERS },
        { 3241, "Buzz Wing", SKY_TYPE_VEHICLE, SKY_ELEMENT_LIFE, SKY_GAME_SUPERCHARGERS },

        // ==================== Adventure Packs (SWAP Force) ====================
        { 3300, "Sheep Wreck Island", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SWAP_FORCE },
        { 3301, "Tower of Time", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SWAP_FORCE },
        { 3302, "Arkeyan Crossbow", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SWAP_FORCE },

        // ==================== SuperChargers - Drivers ====================
        { 3400, "Fiesta", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_UNDEAD, SKY_GAME_SUPERCHARGERS },
        { 3401, "High Volt", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_TECH, SKY_GAME_SUPERCHARGERS },
        { 3402, "Splat", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_MAGIC, SKY_GAME_SUPERCHARGERS },
        { 3406, "Stormblade", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_AIR, SKY_GAME_SUPERCHARGERS },
        { 3411, "Smash Hit", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_EARTH, SKY_GAME_SUPERCHARGERS },
        { 3412, "Steel Plated Smash Hit", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_EARTH, SKY_GAME_SUPERCHARGERS },
        { 3413, "Hurricane Jet-Vac", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_AIR, SKY_GAME_SUPERCHARGERS },
        { 3414, "Double Dare Trigger Happy", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_TECH, SKY_GAME_SUPERCHARGERS },
        { 3415, "Super Shot Stealth Elf", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_LIFE, SKY_GAME_SUPERCHARGERS },
        { 3416, "Shark Shooter Terrafin", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_EARTH, SKY_GAME_SUPERCHARGERS },
        { 3417, "Bone Bash Roller Brawl", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_UNDEAD, SKY_GAME_SUPERCHARGERS },
        { 3420, "Big Bubble Pop Fizz", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_MAGIC, SKY_GAME_SUPERCHARGERS },
        { 3421, "Lava Lance Eruptor", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_FIRE, SKY_GAME_SUPERCHARGERS },
        { 3422, "Deep Dive Gill Grunt", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_WATER, SKY_GAME_SUPERCHARGERS },
        { 3423, "Turbo Charge Donkey Kong", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_LIFE, SKY_GAME_SUPERCHARGERS },
        { 3424, "Hammer Slam Bowser", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_FIRE, SKY_GAME_SUPERCHARGERS },
        { 3425, "Dive-Clops", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_WATER, SKY_GAME_SUPERCHARGERS },
        { 3426, "Astroblast", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_LIGHT, SKY_GAME_SUPERCHARGERS },
        { 3427, "Nightfall", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_DARK, SKY_GAME_SUPERCHARGERS },
        { 3428, "Thrillipede", SKY_TYPE_SUPERCHARGER, SKY_ELEMENT_LIFE, SKY_GAME_SUPERCHARGERS },

        // ==================== Racing Villains ====================
        { 3440, "Chef Pepper Jack", SKY_TYPE_VEHICLE, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },
        { 3441, "Stratosfear", SKY_TYPE_VEHICLE, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },
        { 3442, "Cap'n Cluck", SKY_TYPE_VEHICLE, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },
        { 3443, "Wolfgang (Racing)", SKY_TYPE_VEHICLE, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },
        { 3444, "Threatpack", SKY_TYPE_VEHICLE, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },
        { 3445, "Hood Sickle (Racing)", SKY_TYPE_VEHICLE, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },
        { 3446, "Bad Juju (Racing)", SKY_TYPE_VEHICLE, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },

        // ==================== Trophies ====================
        { 3500, "Sky Trophy", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },
        { 3501, "Land Trophy", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },
        { 3502, "Sea Trophy", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },
        { 3503, "Kaos Trophy", SKY_TYPE_ITEM, SKY_ELEMENT_NONE, SKY_GAME_SUPERCHARGERS },

        // End marker
        { 0xFFFF, nullptr, SKY_TYPE_UNKNOWN, SKY_ELEMENT_NONE, SKY_GAME_UNKNOWN }
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

    const char* skylanderDB::getGameString(SkylanderGame game)
    {
        switch (game)
        {
            case SKY_GAME_SPYROS_ADVENTURE: return "Spyros Adventure";
            case SKY_GAME_GIANTS:           return "Giants";
            case SKY_GAME_SWAP_FORCE:       return "Swap Force";
            case SKY_GAME_TRAP_TEAM:        return "Trap Team";
            case SKY_GAME_SUPERCHARGERS:    return "SuperChargers";
            case SKY_GAME_IMAGINATORS:      return "Imaginators";
            default:                        return "Unknown";
        }
    }

    const char* skylanderDB::getElementString(SkylanderElement element)
    {
        switch (element)
        {
            case SKY_ELEMENT_AIR:    return "Air";
            case SKY_ELEMENT_EARTH:  return "Earth";
            case SKY_ELEMENT_FIRE:   return "Fire";
            case SKY_ELEMENT_WATER:  return "Water";
            case SKY_ELEMENT_MAGIC:  return "Magic";
            case SKY_ELEMENT_TECH:   return "Tech";
            case SKY_ELEMENT_LIFE:   return "Life";
            case SKY_ELEMENT_UNDEAD: return "Undead";
            case SKY_ELEMENT_LIGHT:  return "Light";
            case SKY_ELEMENT_DARK:   return "Dark";
            case SKY_ELEMENT_KAOS:   return "Kaos";
            case SKY_ELEMENT_NONE:   return "None";
            default:                 return "Unknown";
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
