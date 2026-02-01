/*
 * Skylander NFC Key Helpers
 * Copyright (C) 2024 Koos du Preez
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * CREATED BY: Koos du Preez - kdupreez@hotmail.com
 */

#include <string>
#include <stdio.h>
#include <stdint.h>
#include <iomanip>

namespace  xk
{
    #define SKYLANDER_BLOCK_0_KEY_A {0x4B, 0x0B, 0x20, 0x10, 0x7C, 0xCB}
    #define SKYLANDER_BLOCK_0_VENDOR_INFO {0x4B, 0x0B, 0x20, 0x10, 0x7C, 0xCB}
    #define SKYLANDER_BLOCK_0_CARD_INFO {0x08, 0x04}

    class skylanderNFC
    {
        public:
            skylanderNFC() = delete;

            static std::string toHexStr(const uint8_t* data, size_t size) 
            {
                std::stringstream hexStream;
                hexStream << std::uppercase;
                for (size_t i = 0; i < size; ++i) 
                {
                    hexStream << std::hex << std::setfill('0') << std::setw(2)
                            << static_cast<int>(static_cast<unsigned char>(data[i]));
                }
                return hexStream.str();
            }

            // Key Generation Algorithm Translated from Python 
            // https://nfc.toys/interop-sky.html
            static uint64_t makeSkyKey(uint8_t sectorIndex, uint8_t uid[4])
            {
                //setup data for CRC
                uint8_t uid_and_sector[5];
                memcpy(uid_and_sector, uid, 4);
                uid_and_sector[4] = sectorIndex;

                uint64_t poly = 0x42f0e1eba9ea3693ULL;
                uint64_t msb =  0x0000800000000000ULL;
                uint64_t trim = 0x0000ffffffffffffULL;

                //CRC48 Init as magic prime numbers product
                uint64_t CRC48 =  2 * 2 * 3 * 1103 * 12868356821ULL;

                //Compute CRC48
                for (size_t i = 0; i < 5; i++)
                {
                    uint64_t x = uid_and_sector[i];
                    CRC48 =  CRC48 ^ (x << 40);

                    for (uint8_t k = 0; k < 8; k++)
                    {
                        if (CRC48 & msb)
                        CRC48 = (CRC48 << 1) ^ poly;
                        else
                        CRC48 = CRC48 << 1;

                        CRC48 = CRC48 & trim;
                    }
                }

                return CRC48;
            }

    };
}

